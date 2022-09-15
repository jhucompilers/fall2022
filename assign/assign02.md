---
layout: default
title: "Assignment 2"
---

*Note: this is a preliminary version of the assignment description, which
has information about required lexer and parser changes, but is still
missing most details about semantics such as implementing functions.
It will be updated. Note that the [Lecture 5 slides](../lectures/lecture05-public.pdf)
do have a relatively detailed presentation of functions and function
calls.*

**Due**: TBD

# Interpreter part 2: Functions, control structures, runtime

This assignment is a continuation of [Assignment 1](assign01.html), in which
you will add support for functions and control structures to turn the
interpreter into a full programming language somewhat reminiscent of
[JavaScript](https://en.wikipedia.org/wiki/JavaScript).

## Getting started

Start by making a copy of your code for Assignment 1. Assuming your
code is in a directory called `assign01`, you could use a command like

```
cp -r assign01 assign02
```

You could also just modify your existing code for Assignment 1.

Regardless of which approach you use, be sure to use version control
(e.g., [Git](https://git-scm.com/)) to record and preserve your
development progress.

## Grading criteria

The grading criteria are as follows:

* Implementation of functions and function calls (including intrinsic functions): 83%
* Arrays: 4%
* Strings: 2%
* Lambdas: 1%
* Design and coding style: 10%

Note that it is assumed that all of the functionality from
[Assignment 1](assign01.html) works correctly.

## Lexical analyzer changes

You will need to add the following new keyword tokens:

```
function
if
else
while
```

You will also need to add the following grouping and sequencing tokens:

```
{
}
,
```

For the sake of clarity, the new grouping and sequencing tokens are
left brace (`{`), right brace (`}`), and comma (`,`).

## Parser changes

You will need to change the productions

<div class='highlighter-rouge'><pre>
Unit &rarr; Stmt
Unit &rarr; Stmt Unit
</pre></div>

to

<div class='highlighter-rouge'><pre>
Unit &rarr; TStmt
Unit &rarr; TStmt Unit
</pre></div>

The "`TStmt`" nonterminal means "top-level statement", which means statements
that can appear in the global scope.

You will also need to implement the following new productions
(each occurrence of blue italized text beginning with <code><i style="color:blue;">--</i></code>
represents an explanatory comment, and is not part of the production):

<div class='highlighter-rouge'><pre>
TStmt &rarr;      Stmt
TStmt &rarr;      Func
Stmt &rarr;       if ( A ) { SList }                        <i style="color:blue;">-- if statement</i>
Stmt &rarr;       if ( A ) { SList } else { SList }         <i style="color:blue;">-- if/else statement </i>
Stmt &rarr;       while ( A ) { SList }                     <i style="color:blue;">-- while loop</i>
Func &rarr;       function ident ( OptPList ) { SList }     <i style="color:blue;">-- function definition</i>
OptPList &rarr;   PList                                     <i style="color:blue;">-- optional parameter list</i>
OptPList &rarr;   ε
PList &rarr;      ident                                     <i style="color:blue;">-- nonempty parameter list</i>
PList &rarr;      ident , PList
SList &rarr;      Stmt                                      <i style="color:blue;">-- statement list</i>
SList &rarr;      Stmt SList
F &rarr;          ident ( OptArgList )                      <i style="color:blue;">-- function call</i>
OptArgList &rarr; ArgList                                   <i style="color:blue;">-- optional argument list</i>
OptArgList &rarr; ε
ArgList &rarr;    L                                         <i style="color:blue;">-- nonempty argument list</i>
ArgList &rarr;    L , ArgList
</pre></div>

Note that the <code class='highlighter-rouge'>OptPList &rarr; ε</code> and
<code class='highlighter-rouge'>OptArgList &rarr; ε</code> productions are
*epsilon productions*, meaning that there are no symbols on the right
hand side of the production, so they represent a step in the derivation
where the nonterminal symbol on the left hand side expands to nothing.

As you implement these productions, you should think about how to best
represent the constructs in the resulting AST. As with Assignment 1,
you should try to structure the AST in a way that will make it
easy for the interpreter to analyze and evaluate.  As a concrete example,
consider the following input (which is [`function02.in` in the
test repository](https://github.com/jhucompilers/fall2022-tests/blob/main/assign02/input/function02.in)):

```
var g;
g = 42;

function f(x) {
  x + g;
}

function q(x) {
  var g;
  g = 17;
  f(x);
}

q(1);
```

Here is a test run showing how the reference solution turns this input
into an AST (user input in **bold**):

<div class='highlighter-rouge'><pre>
$ <b>$ASSIGN02_DIR/minilang -p input/function02.in</b>
UNIT
+--STATEMENT
|  +--VARDEF
|     +--VARREF[g]
+--STATEMENT
|  +--ASSIGN
|     +--VARREF[g]
|     +--INT_LITERAL[42]
+--FUNCTION
|  +--VARREF[f]
|  +--PARAMETER_LIST
|  |  +--VARREF[x]
|  +--AST_STATEMENT_LIST
|     +--STATEMENT
|        +--ADD
|           +--VARREF[x]
|           +--VARREF[g]
+--FUNCTION
|  +--VARREF[q]
|  +--PARAMETER_LIST
|  |  +--VARREF[x]
|  +--AST_STATEMENT_LIST
|     +--STATEMENT
|     |  +--VARDEF
|     |     +--VARREF[g]
|     +--STATEMENT
|     |  +--ASSIGN
|     |     +--VARREF[g]
|     |     +--INT_LITERAL[17]
|     +--STATEMENT
|        +--FNCALL
|           +--VARREF[f]
|           +--ARGLIST
|              +--VARREF[x]
+--STATEMENT
   +--FNCALL
      +--VARREF[q]
      +--ARGLIST
         +--INT_LITERAL[1]
</pre></div>

This is intended as an example. You may structure the AST in whatever way
seems most appropriate.

## Semantics of functions and function calls

*Coming soon!*

## Values and reference counting of dynamic representations

*Coming soon!*

## Intrinsic functions

*Coming soon!*

## Arrays and array intrinsic functions

*Coming soon!*

## Strings and string intrinsic functions

*Coming soon!*

## Lambdas (anonymous functions)

*Coming soon!*

## Submitting

*Coming soon!*
