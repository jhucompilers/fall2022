---
layout: default
title: "Assignment 3"
---

**Due date**: Friday, Oct 21st by 11pm Baltimore time

# Compiler: semantic analysis

*Note: if you are reading this, understand that this assignment description
has not been announced publically yet. Important details could
conceivably change before that happens.*

*Note: this assignment description is preliminary, but includes enough
information to start the assignment and make substantial progress.
This description will be updated, and I will announce in class and
on Courselore when that happens.*

In this assignment you will implement semantic analysis and type
checking for a compiler for a subset of the C programming language.

## Getting started

Download [assign03.zip](assign03.zip) and unzip it.

To compile the program, use the commands

```
make depend
make -j
```

You will be (primarily) modifying the source files `semantic_analysis.h` and
`semantic_analysis.cpp`. Note that you are free to modify other source files as
necessary.

In this assignment, the expected output of the executable
(which is called "`nearly_cc`") is a textual representation of the symbol
table entries created by the semantic analyzer, or (if appropriate)
an error message printed to `stderr`.  The executable should be invoked
using a command of the form

<div class="highlighter-rouge"><pre>
$ASSIGN03_DIR/nearly_cc -a <i>sourcefile</i>
</pre></div>

where `$ASSIGN03_DIR` expands to the directory containing your work, and
*sourcefile* is the C source file to analyze.

The "`-a`" command line option stands for "analyze."

## Your task

Your task is to implement the node visitation member functions in
the `SemanticAnalysis` class so that

* symbol tables are created and populated for the input source program, and
* the input source is checked for semantic errors, which should be reported
  using the `SemanticError::raise` function

## Visualizing the AST

Your semantic analyzer will need to work with the AST representation of the input
code. We strongly recommend that you make use of the `-p` command line option
to have your `nearly_cc` program print a text representation of the AST. This will
give you detailed knowledge of how the AST is represented for whichever part of
the AST you are working on.  For example:

<div class='highlighter-rouge'><pre>
$ <b>cat input/example01.c</b>
int main(void) {
  return 0;
}
$ <b>$ASSIGN03_DIR/nearly_cc -p input/example01.c</b>
AST_UNIT
+--AST_FUNCTION_DEFINITION
   +--AST_BASIC_TYPE
   |  +--TOK_INT[int]
   +--TOK_IDENT[main]
   +--AST_FUNCTION_PARAMETER_LIST
   +--AST_STATEMENT_LIST
      +--AST_RETURN_EXPRESSION_STATEMENT
         +--AST_LITERAL_VALUE
            +--TOK_INT_LIT[0]
</pre></div>


You might also find it helpful to refer to
`parse_buildast.y`, which is the Bison parser, since its semantic actions are
responsible for creating the AST.

## Testing

As with Assignments 1 and 2, a public test suite is provided to serve as a
detailed specification of the expected functionality of your program.

To use the test suite, start by setting the `ASSIGN03_DIR` environment
variable to the path of the directory containg your work, e.g.

```
export ASSIGN03_DIR=~/compilers/assign03
```

assuming that `~/compilers/assign03` is the directory containing your code.

The test inputs are in the "`input`" directory.  To run a test, use the
`run_test.rb` script as follows, specifying the base name of the test you want
to run:

```
./run_test.rb example01
```

The command above will test your semantic analyzer on the test input
called `input/example01.c`.

You can see the actual output and/or error message produced by your program
in the `actual_output` and `actual_error` directories. You can compare these
to the expected output and error in the `expected_output` and `expected_error`
directories.

## Detailed requirements, specifications, and advice

This will be a fairly complex assignment, but if you approach it methodically
you should be able to make steady progress.

### Suggested approach

One way to get started is to work on the `visit_basic_type` member function.
This function should analyze the basic type and type qualifier keywords,
and then create a `BasicType` object (wrapped in a `std::shared_ptr<Type>` smart
pointer) to represent the basic type. Of course, if the combination of
keywords doesn't identify a valid basic type (e.g., `long char`)
then use `SemanticError::raise` to report the error.
If the basic type is valid, annotate its AST node with a (shared)
pointer to the type object.

Once basic types are handled, you could tackle variable declarations.
You will need to first process the base type, then (based on the type
representation determined for the base type) you can work your way
through the declarators and add each one as a symbol table entry (since
each one will be defining a variable.)

If you're at the point where declarations like

```c
unsigned int x, y[10], *z;
```

are working well, then you're off to a very good start.

Analyzing expressions should be fairly straightforward, as long as you can
handle the leaf nodes, which will be `AST_VAR_REF` and the various kinds of
literals. Each leaf should be annotated with (at least) a type. Variable reference
nodes should be annotated with a pointer to the `Symbol` representing the
symbol table entry for the referred-to variable or function.

### SemamticAnalysis class, ASTVisitor

The `SemanticAnalysis` class derives from `ASTVisitor`. The `Context::analyze`
function invokes the `visit` member function on the instance of
`SemanticAnalysis`, passing the root node of the AST.  This begins
a traversal of the AST.  Starting with the root node, which has the
tag `AST_UNIT`, the appropriate `visit_` member function will be
called for each visited node in the AST. Because the default behavior
of each visit member function is to recursively visit children,
this effects a traversal of the AST.

Note, however, that certain `visit_` member functions are overridden.
These are the ones which (at a minimum) you will need to implement to
perform semantic analysis.

The goal of semantic analysis is to build symbol tables recording what
each name used in the input refers to (variables, functions, and struct
types), as well as annotating AST nodes with pointers to symbol table
entries (`Symbol` objects) or types (`std::shared_ptr<Type>` objects
pointing to a reference-counted instance of the `Type` class)
as appropriate.

`AST_VAR_REF` nodes should be annotated with a pointer to the symbol
table entry for the variable or function the name refers to.
(Note that `AST_VAR_REF` nodes *can* refer to functions.)
For all expressions that are not variable references, the node should
be annotated with a type.

You will need to add fields and member functions to the `NodeBase` class
to store these annotations.  Note that if an `AST_VAR_REF` node is annotated with
a pointer to a `Symbol` object, the `Symbol` object contains a shared
pointer to the variable (or function's) type, so it isn't necessary
to represent the type separately.  Here is a possible approach to
adding member functions to the `NodeBase` class to support symbol and
type annotations:

```c++
void NodeBase::set_symbol(Symbol *symbol) {
  assert(!has_symbol());
  assert(m_type == nullptr);
  m_symbol = symbol;
}

void NodeBase::set_type(const std::shared_ptr<Type> &type) {
  assert(!has_symbol());
  assert(!m_type);
  m_type = type;
}

bool NodeBase::has_symbol() const {
  return m_symbol != nullptr;
}

Symbol *NodeBase::get_symbol() const {
  return m_symbol;
}

std::shared_ptr<Type> NodeBase::get_type() const {
  // this shouldn't be called unless there is actually a type
  // associated with this node

  if (has_symbol())
    return m_symbol->get_type(); // Symbol will definitely have a valid Type
  else {
    assert(m_type); // make sure a Type object actually exists
    return m_type;
  }
}
```

This code assumes that the `m_symbol` member variable has the type `Symbol *`
and that the `m_type` member variable has the type `std::shared_ptr<Type>`.
You are free to use or adapt this code, but you are not required to.

### Symbol and SymbolTable classes

The `SymbolTable` class plays more or less the same role in the compiler
as the `Environment` class played in [Assignment 1](assign01.html) and
[Assignment 2](assign02.html). Specifically, it records information
about each named variable, function, and struct type in the program.
Unlike `Environment`, it is not a runtime data structure, because
the compiler will be generating x86-64 assembly language code to carry
out the execution of the program.

A `SymbolTable` is a collection of *symbol table entries*, which are
represented by dynamically-allocated instances of the `Symbol` class.
There are three kinds of symbol table entries (defined by the
`SymbolTableKind` enumeration type): variables, functions, and types.
Type entries are only used for struct data types.

Like the interpreter language in Assignments 1 and 2, C is a block
scoped language. Each statement list (represented by `AST_STATEMENT_LIST`
nodes in the AST) is a scope nested within its parent scope.
It is not allowed to define to variables or functions with the same
name in the same scope. However, it is allowed to define a variable
in an inner scope whose name is the same as a variable in an outer
(enclosing) scope.

Each `SymbolTable` has a link to its "parent" `SymbolTable`, representing
the enclosing scope. The `SymbolTable` representing the global scope
does not have a parent.

### Type representations

*Coming soon!*

### LiteralValue

*Coming soon!*

### C semantic rules

Note that the [slides for Lecture 11](../lectures/lecture11-public.pdf) are a very
good overview of the semantic rules your semantic analyzer is expected to
check.

*Coming soon!*
