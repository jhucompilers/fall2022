---
layout: default
title: "Assignment 3"
---

**Due date**: Friday, Oct 21st by 11pm Baltimore time

# Compiler: semantic analysis

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

## Testing

As with Assignments 1 and 2, a public test suite is provided to serve as a
detailed specification of the expected functionality of your program.

To use the test suite, start by setting the `ASSIGN03_DIR` environment
variable to the path of the directory containg your work, e.g.

```
export ASSIGN03_DIR=~/compilers/assign03
```

assuming that `~/compilers/assign3` is the directory containing your code.

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

*Coming soon!*

### Type representations

*Coming soon!*

### LiteralValue

*Coming soon!*

### C semantic rules

Note that the [slides for Lecture 11](../lectures/lecture11-public.pdf) are a very
good overview of the semantic rules your semantic analyzer is expected to
check.

*Coming soon!*
