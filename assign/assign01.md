---
layout: default
title: "Assignment 1"
---

Due: Friday, Sep 16 by 11pm Baltimore time

# Interpreter part 1: expression evaluation

In this assignment you will implement an interpreter which serves
as a calculator, reading a sequence of statements containing expressions
to be evaluated, and then analyzing and evaluating them.

## Getting started

Download [assign01.zip](assign01.zip) and unzip it:

```
wget https://jhucompilers.github.io/fall2022/assign/assign01.zip
unzip assign01.zip
```

These commands will create a directory called `assign01` which
contains the starter code.

## Lexical analyzer changes

You will need to implement changes to the lexical analyzer (in `lexer.cpp`,
as well as `token.h`) to add the following new tokens:

```
var
=
||
&&
<
<=
>
>=
==
!=
```

Note that the two-character tokens where the first character is also
a valid token — for example, `>=` — will require special handling.
You might want to add new helper functions to the lexer.

## Parser changes

These are the grammar productions implemented in the starter code
(which is adapted from the [astdemo](https://github.com/daveho/astdemo)
example code):

<div class="highlighter-rouge"><pre>
Unit &rarr; Stmt
Unit &rarr; Stmt Unit
Stmt &rarr; E ;
E    &rarr; T E'
E'   &rarr; + T E'
E'   &rarr; - T E'
E'   &rarr; ε
T    &rarr; F T'
T'   &rarr; * F T'
T'   &rarr; / F T'
T'   &rarr; ε
F    &rarr; number
F    &rarr; ident
F    &rarr; <code>(</code> E <code>)</code>
</pre></div>

You will add the following productions to the grammar (implement these
in `Parser2`):

<div class="highlighter-rouge"><pre>
Stmt &rarr; <code>var</code> ident <code>;</code>
A    &rarr; ident <code>=</code> A
A    &rarr; L
L    &rarr; R <code>||</code> R
L    &rarr; R <code>&amp;&amp;</code> R
L    &rarr; R
R    &rarr; E <code>&lt;</code> E
R    &rarr; E <code>&lt;=</code> E
R    &rarr; E <code>&gt;</code> E
R    &rarr; E <code>&gt;=</code> E
R    &rarr; E <code>==</code> E
R    &rarr; E <code>!=</code> E
R    &rarr; E
</pre></div>

Note that the symbols beginning with an upper-case letter
(e.g., `Stmt`, `A`, `L`, `R`, `E`, `E'`, etc.) are nonterminals, and all other symbols
(e.g., `(`, `)`, `=`, `==`, `<`, etc.) are terminals.

You will also need to change the production

<div class="highlighter-rouge"><pre>
Stmt &rarr; E ;
</pre></div>

to

<div class="highlighter-rouge"><pre>
Stmt &rarr; A ;
</pre></div>

You will need to add new AST node types to `ast.h` and `ast.cpp`
to represent the new constructs added to the language.
Specifically:

* The assignment (`=`) operator
* The logical and (`&&`) and or (`||`) operators
* The comparison (`<`, `<=`, `>`, `>=`) operators
* The equality and inequality (`==`, `!=`) operators

### Parsing assignments

The <code class='highlighter-rouge'>A &rarr; ident = A</code> production requires two
tokens of lookahead. The reason is that if the parser is
trying to expand an `A` nonterminal symbol, and the next token
is an identifier, there is not enough information to know
whether `A` will expand to an assignment or to a different kind
of expression.  However, if the identifier is immediately followed by
an assignment operator (`=`), that indicates that `A`
should expand to an assignment.

The `Lexer` class's `peek()` member function takes an optional
argument to specify how many tokens ahead to peek. For example:

```c++
Node *next_tok = m_lexer->peek(1);
Node *next_next_tok = m_lexer->peek(2);
```

Note that if input ends before the lexer can scan the requested
number of tokens, `peek()` returns a null pointer.

## Testing your lexer and parser changes

Once you have implemented the changed described above, you can run the
program and have it print a textual representation of the AST constructed
from the input.  For example, let's say that the input file `t/example02.txt`
has the following contents:

```
var a;
var b;
a = 11;
b = 22;
var c;
c = (a * 4 < 55) && (b == 16 || b >= 9 + 13);
c;
```

Run the program (after compiling it) as follows:

```
./minilang -p t/example02.txt
```

This invocation could produce the following output:

```
UNIT
+--STATEMENT
|  +--VARDEF
|     +--VARREF[a]
+--STATEMENT
|  +--VARDEF
|     +--VARREF[b]
+--STATEMENT
|  +--ASSIGN
|     +--VARREF[a]
|     +--INT_LITERAL[11]
+--STATEMENT
|  +--ASSIGN
|     +--VARREF[b]
|     +--INT_LITERAL[22]
+--STATEMENT
|  +--VARDEF
|     +--VARREF[c]
+--STATEMENT
|  +--ASSIGN
|     +--VARREF[c]
|     +--LOGICAL_AND
|        +--LT
|        |  +--MULTIPLY
|        |  |  +--VARREF[a]
|        |  |  +--INT_LITERAL[4]
|        |  +--INT_LITERAL[55]
|        +--LOGICAL_OR
|           +--EQ
|           |  +--VARREF[b]
|           |  +--INT_LITERAL[16]
|           +--GTE
|              +--VARREF[b]
|              +--ADD
|                 +--INT_LITERAL[9]
|                 +--INT_LITERAL[13]
+--STATEMENT
   +--VARREF[c]
```

Note that there is no inherently correct or incorrect form for the AST
corresponding to a particular input, other than representing the
semantic properties of the input (such as order of evaluation)
correctly. You should arrange the code in the parser to construct an AST that

1. Embodies the essential information about the input that wil be
   needed for interpretation, and
2. Is as simple as possible

## Interpreting the AST

After implementing the changes to the lexer and parser described above,
you will implement the `analyze()` and `execute()` member functions
in the `Interpreter` class.

The idea is that the `analyze()` function will inspect the AST and
verify that it does not have semantic errors that would prevent
interpretation. The only required semantic check for this milestone
is verifying that all variable references are preceeded by a
variable definition. If the AST does not have any semantic errors,
`analyze()` returns normally. If there are any semantic errors,
it should use the `SemanticError::raise` function to throw a
`SemanticError` exception.

The `execute()` function should iterate through the statements and evaluate
each one.  A variable definition statement should create the named
variable and initialize it with the value 0. For statements containing
an expression, the expression should be evaluated. If there are any
assignments in the expression, the value of the variable on the left hand side of
the assignment should be updated to store the computed value of the
expression on the right hand side of the assignment.

The `execute()` function should return the value that results from
evaluating the last statement. Note that as a special case, evaluating
variable definition produces the value 0.

## Evaluating operators

Expression evaluation is recursive. The base cases are integer literals,
which "self-evaluate", and variable references, where the value of the
variable reference is found by looking it up in the environment.
(The `Environment` class embodies an environment, which for this milestone
is a map of variable names to their values.) To evaluate an operator,
first recursively evaluate the subexpressions, then perform the operation
on the resulting value.

All evaluations should use the `Value` type to represent values.
This type (as implemented in the starter code) uses the `int`
data type as the representation of numeric values.

Operators should be evaluated as follows:

Operator | How to evaluate
:------: | ---------------
`+`      | Compute sum
`-`      | Compute difference
`*`      | Compute product
`/`      | Compute quotient: throw an `EvaluationError` if the divisor is 0
`&&`     | 1 if both operands a non-zero, 0 if either operand is zero
`||`     | 1 if either operand is non-zero, 0 if both operands are zero
`>`      | 1 if left operand is greater than right operand, 0 otherwise
`>=`     | 1 if left operand is greater than or equal to right operand, 0 otherwise
`<`      | 1 if left operand is less than right operand, 0 otherwise
`<=`     | 1 if left operand is less than or equal to right operand, 0 otherwise
`==`     | 1 if operands are equal, 0 otherwise
`!=`     | 1 if operands are not equal, 0 otherwise

## Testing

*(Note: the test repository is currently incompletely. We will be updating
it to add many more tests in the near future.)*

The [compilers-fall2022-tests](https://github.com/jhucompilers/fall2022-tests)
repository has some tests you can use to check your implementation.
First, clone the test repository:

```
git clone https://github.com/jhucompilers/fall2022-tests.git compilers-fall2022-tests
```

We will refer to the directory name of your clone of the test repository as
`$TEST_REPO`.

To run tests, first set the `ASSIGN01_DIR` environment variable to
the directory containing your program (and in particular, the `minilang`
executable):

```
export ASSIGN01_DIR=~/compilers/assign01
```

The above command assumes that `~/compilers/assign01` is the directory containing
your work. Adjust this as necessary.

Next, change directory to `assign01` subdirectory of the test repository:

```
cd $TEST_REPO/assign01
```

To run a single test, use the `run_test.rb` script. For example,

```
./run_test.rb example01
```

will execute the `example01` test. To run all tests, use the command

```
./run_all.rb
```

We encourage you to add your own tests. The `input` directory contains
the test programs to be evaluated by the `minilang` interpreter program.
Test programs should end in the `.in` file extension. The `expected_output`
directory contains the expected output for the test program. Expected
output files should end in the `.out` file extension, and the stem of
filename should match the stem of the corresponding test program.
The `expected_error` directory contains the expected error output,
also ending with the `.out` file extension and with the stem matching
the test program. If no error output is expected, this file can be
omitted.

Here is an example showing how to create a very simple test (these
commands assume your shell is in the `$TEST_REPO/assign01` directory):

```
echo "1 + 2;" > input/example01.in
echo "Result: 3" > expected_output/example01.out
./run_test.rb example01
```

Assuming that your `minilang` program can evaluate the expression `1 + 2`
correctly, the `run_test.rb` script should produce the output

```
Test passed!
```

## Non-functional requirements

We expect your code to be clean, readable, well-designed, and
(reasonably) efficient.  Please refer to the
[design, coding style, and efficency](design.html) guidelines.

We also expect your code to be free of runtime errors such as
uses of uninitialized variables, out of bounds array accesses,
and memory leaks. You can and should use `valgrind` when testing
your code to check for such errors. We also highly recommend using
[`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr)
to manage pointers to dynamically-allocated objects (to ensure
that they are deallocated when no longer needed.)

You should submit a `README.txt` file that briefly explains your
implementation. A paragraph or two is sufficient. If you used any
interesting implementation techniques, let us know about them.
Also, if there are any limitations such as features that you weren't
able to get working, you can document them here.

## Submitting

To submit, create a zipfile with your code, the `Makefile`, and the
`README.txt`. Suggested command:

```
zip -9r solution.zip *.h *.cpp Makefile README.txt
```

Upload the zipfile (i.e., `solution.zip`) to [Gradescope](https://www.gradescope.com)
as **Assignment 1**.
