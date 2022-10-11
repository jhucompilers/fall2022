---
layout: default
title: "Assignment 3"
---

**Due date**: Monday, Oct 24th by 11pm Baltimore time

*Update 10/11*: Updated the section on [Struct types](#struct-types).

# Compiler: semantic analysis

*Note: this assignment description is somewhat preliminary, but includes enough
information to start the assignment and make substantial progress.
This description and the public tests will be updated, and I will announce
in class and on Courselore when that happens.*

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
`semantic_analysis.cpp`. You will also almost certainly need to modify the
`NodeBase` class (defined in `node_base.h` and `node_base.cpp`) to add
data members and member functions. Note that you are free to modify other source files if
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

As with Assignments 1 and 2, **a public test suite is provided to serve as a
detailed specification of the expected functionality of your program**.
(I've emphasized this text because I want to emphasize that the public tests
will probably be at least as useful as a functional specification as the text
in this assignment description.)

To use the test suite, start by setting the `ASSIGN03_DIR` environment
variable to the path of the directory containg your work, e.g.

```
export ASSIGN03_DIR=~/compilers/assign03
```

assuming that `~/compilers/assign03` is the directory containing your code.

You will want to change directory into the directory `assign03` of
your clone of [the public test repository](https://github.com/jhucompilers/fall2022-tests).

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
pointer) to represent the basic type. If either of the type qualifiers
were specified, then the `BasicType` can be wrapped in a `QualifiedType`.)
Of course, if the combination of keywords doesn't identify a valid basic
type (e.g., `long char`) then use `SemanticError::raise` to report the error.
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

You can use the `define` and `declare` member functions of `SymbolTable`
to create a new symbol table entry.  `define` should be used for
function definitions and all variable declarations. `declare` should
be used only for function declarations (a.k.a. function prototypes.)

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

You might find the following helper functions to be useful for entering
and leaving scopes in the source program:

```c++
void SemanticAnalysis::enter_scope() {
  SymbolTable *scope = new SymbolTable(m_cur_symtab);
  m_cur_symtab = scope;
}

void SemanticAnalysis::leave_scope() {
  m_cur_symtab = m_cur_symtab->get_parent();
  assert(m_cur_symtab != nullptr);
}
```

### Type representations

As discussed in [Lecture 11](../lectures/lecture11-public.pdf), trees are a
good way to represent C data types. The starter code defines a base class
called `Type`, with concrete derived classes `BasicType`, `QualifiedType`,
`PointerType`, `ArrayType`, `StructType`, and `FunctionType`.

You should read through the header file `type.h` to familiarize yourself with
these types and the operations they support.  Note that the base class
`Type` defines a "wide" interface of operations, meaning that any operation
that would be meaningful for any of the derived classes is defined in the
base class. This makes `Type` objects easy to work with, but it also raises
the possibility that your program could invoke an operation that is not
meaningful. For example, if your semantic analyzer calls the `get_num_members()`
function on an instance of `BasicType`, an exception will be thrown.

Because of the way that C variable declarations are split into a base type
and declarators, it is natural to want to allow type representations to
share parts of the representation. For this reason, `Type` objects
(or, more specifically, objects that are derived from `Type`) are meant
to be wrapped in `std::shared_ptr<Type>` smart pointers. This allows
`Type` objects to be reference counted, so that the object is deleted when
the last reference to it disappears.

To make the reference counting work correctly, you should follow these
rules:

1. When a type object is dynamically allocated, it should *immediately*
   be wrapped in a `std::shared_ptr<Type>`
2. All further references to the object should also be managed by
   `std::shared_ptr<Type>` instances created via copying or assignment

You should *not* ever allow two "unrelated" `std::shared_ptr<Type>` instances
to be created from the same "raw" pointer. The following code illustrates
the correct and incorrect ways to create and refer to `Type` objects.

```c++
// a new type object should be immediately wrapped by a shared_ptr
std::shared_ptr<Type> uchar_type(new BasicType(BasicTypeKind::CHAR, false));

std::shared_ptr<Type> copy1(uchar_type); // good
std::shared_ptr<Type> copy2;
copy2 = uchar_type;                      // this is also fine

Type *ushort_type = new BasicType(BasicTypeKind::SHORT, true); // dangerous raw pointer

std::shared_ptr<Type> copy3(ushort_type); // problematic
std::shared_ptr<Type> copy4(ushort_type); // problematic
```

In the code above, `copy3` and `copy4` will use different (unrelated) reference counts
for the type object, leading to a high likelihood that the type object will be
destroyed at a point when there are still `shared_ptr` objects referring to it.

### Struct types

Handling the definition of a struct type will require a somewhat specific approach,
which is described here. (This is a guide to implementing the
`SemanticAnalysis::visit_struct_type_definition` member function.)

To start with, an instance of `StructType` should be created, and immediately
entered into the current symbol table.  This code could look something like
the following:

```c++
std::string name = /* the name of the struct type */;
std::shared_ptr<Type> struct_type(new StructType(name));

m_cur_symtab->define(SymbolTableKind::TYPE, "struct " + name, struct_type);
```
The reason that the struct type will need to be entered into the symbol
table of the current scope immediately is that struct types can refer to
themselves when defining a recursive data type, such as a linked list node:

```c
struct Node {
  int data;
  struct Node *next;
};
```

In the example above, the base type of the `next` member is pointer to
`struct Node`, so for this declaration to be legal, that type must
already exist in the symbol table for the enclosing scope.

Your semantic analyzer will need to treat the body
of a struct type as being a scope containing variable definitions, where the
variable definitions are the members (fields) of the struct type.
This should be very similar to how a statement list is handled.

Once all of the members have been processed, you can use the
`add_member` member function to add a `Member`
to the instance of `StructType` representing each field.
Once that is done, the representation of the struct type is complete.

Note that it is important to make sure that the names of struct types don't
conflict with the names of variables and functions. Prepending
"`struct `" to the name recorded in the symbol table entry (as shown
above) is a simple way to accomplish this, and is also the approach
that is expected by the public tests.

### LiteralValue

The `LiteralValue` class (defined in `literal_value.h` and `literal_value.cpp`)
is intended to help the compiler

1. decode the lexemes of literal values (integers, characters, and strings)
2. represent literal values

One place where you may find `LiteralValue` to be useful is in determining
the size of an array when the semantic analyzer sees an array declarator.
When later on you implement code generation, `LiteralValue` will likely be
useful for representing integer, character, and string constant values.

### C semantic rules

Note that the [slides for Lecture 11](../lectures/lecture11-public.pdf) are a very
good overview of the semantic rules your semantic analyzer is expected to
check.

**Note also**: to a large degree, the tests in the public test repository are
the best specification of the behavior expected of your semantic analyzer.
For all of the important semantics specified here, you should find at least
one related test.

**Rules for operators**:

If in a use of the `+` and `-` operators one operand is a pointer and the
other is an integer (belonging to any integer data type), then it is performing
pointer arithmetic, and the result type is the same type as the type of the
pointer operand.  The integer operand should be promoted to `int` or `unsigned int`
if its representation is less precise.

Arithmetic on two pointers is never legal.

For arithmetic on two integers (`+`, `-`, `*`, etc.), the following rules apply:

1. If either operand is has a type less precise than `int` or
   `unsigned int`, it is promoted to `int` or `unsigned int`
2. If one operand is less precise than the other, it is promoted
   to the more precise type
3. If the operands differ in signedness, the signed operand is
   implicitly converted to unsigned

For unary (one operand) operations on integer values, the operand value
should be promoted to `int` or `unsigned int` if it belongs to a less-precise
type.

**Assignments**:

An assignment is not legal if the type of the left hand operand is
qualified as `const`.

The left hand side of an assignment must be an *lvalue*.

An *lvalue* is

* a reference to a variable
* an array subscript reference
* a pointer dereference
* a reference to a struct instance
* a reference to a field of a struct instance

If the types of the left and right sides of an assignment are both
integer (`char`, `short`, `int`, etc.) then the assignment is legal.
The right hand type is implicitly converted to the left hand (lvalue)
type.

If the left and right sides are both pointers, then the assignment is
legal if and only if

1. the unqualified base types of each pointer type are identical, and
2. the base type on the left hand side does not lack any qualifiers
   that the base type on the right hand side has

For example, the following code is legal:

```c
char a;
char *right;
right = &a;
const char *left;
left = right; // a legal assignment
```

In the code above, `left`'s base type is `const char` and `right`'s base type
is `char`. The unqualified base types are both `char`, an exact match.
`right`'s base type does not have any qualifiers that `left`'s base type has.

The following code is *not* legal:

```c
const char a;
const char *right;
right = &a;
char *left;
left = right; // illegal: discards "const" qualifier from base type
```

An assignment involving both pointer and non-pointer operands is never legal.

If the left and right sides of an assignment both have a struct type,
the assignment is legal as long as the type of both the left and right
sides are the same struct type.

**Function calls**:

A function call is legal if

* the name of the called function refers to a function that was previously
  either declared or defined
* the number of arguments passed is equal to the function type's number of
  parameters
* each argument expression has a type that could be legally assigned to
  the corresponding parameter (according to the rules for assignment)

The result of a function call has the type indicated by the function's
return value.

**Literals**:

The type of an integer literal defaults to `int`. If its suffix
contains `U` or `u`, then it is unsigned.  If its suffix contains
`L` or `l`, then it is `long`. If both are specified, it is
`unsigned long`. The [LiteralValue class](#literalvalue) has member
functions `is_unsigned()` and `is_long()` to help you decode
and work with integer literals.

The type of a character literal is `int`.

The type of a string literal is `const char *` (pointer to `const char`.)

### Representing implicit conversions

Because code generation will need to know which implicit conversions are needed,
it's a good idea to explicitly represent them in the AST.

The `AST_IMPLICIT_CONVERSION` node tag is intended to allow explicit representation
of implicit conversions. For example, let's say that in checking the left operand
of an operator with two integer operands, the left operand's type is less precise
than `int`, and needs to be promoted. That code could look something like this:

```c++
Node *left = n->get_kid(1);

if (left->get_type()->get_basic_type_kind() < BasicTypeKind::INT)
  n->set_kid(1, left = promote_to_int(left));
```

The `promote_to_int` member function could be implemented this way:

```c++
Node *SemanticAnalysis::promote_to_int(Node *n) {
  assert(n->get_type()->is_integral());
  assert(n->get_type()->get_basic_type_kind() < BasicTypeKind::INT);
  std::shared_ptr<Type> type(new BasicType(BasicTypeKind::INT, n->get_type()->is_signed()));
  return implicit_conversion(n, type);
}

Node *SemanticAnalysis::implicit_conversion(Node *n, const std::shared_ptr<Type> &type) {
  std::unique_ptr<Node> conversion(new Node(AST_IMPLICIT_CONVERSION, {n}));
  conversion->set_type(type);
  return conversion.release();
}
```

Although these implicit conversion nodes aren't strictly needed for this assignment,
they will very likely be useful for code generation in assignments 4 and 5.

## `README.txt`

Please submit a `README.txt` with your submission which briefly discusses anything
interesting or significant you want us to know about your implementation.
If there are any features you weren't able to get fully working, or if you implemented
extra functionality, this is a place you could document that.

## Submitting

Create a zipfile with all of your code, your `Makefile`, and your `README.txt`.
Example commands:

```
make clean
zip -9r solution.zip *.h *.cpp *.rb Makefile README.txt
```

Upload the zipfile to Gradescope as **Assignment 3**.
