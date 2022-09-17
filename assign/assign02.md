---
layout: default
title: "Assignment 2"
---

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

* Control flow (if, if/else, while statements): 33%
* Implementation of functions and function calls (including intrinsic functions): 50%
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
(each occurrence of blue italicized text beginning with <code><i style="color:blue;">--</i></code>
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
of the input string where the nonterminal symbol on the left hand side
expands to nothing.

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

## Interpreter semantics

This section describes the semantics of the new programming language
constructs introduced by the lexer and parser changes.

Note that these semantics, and the required implementation techniques,
are covered in [Lecture 5 slides](../lectures/lecture05-public.pdf),
so the slides will also be a useful reference.

### Global environment

As with [Assignment 1](assign01.html), there should be a global `Environment`
object in which the names of global variables, functions, and intrinsic functions
are bound to their values.

### Control flow

The new control-flow statements are `if`, `if`/`else`, and `while` statements.

When evaluating an `if` statement, the interpreter should first evaluate the
condition. If the result of the evaluation of the condition is not a numeric
(integer) value, an `EvaluationError` should be raised.  Otherwise, if the
result of evaluating the condition is a non-zero value, the body of the
`if` statement should be evaluated.

An `if`/`else` statement is handled the same way as an `if` statement, except
that if the condition evaluates to zero, the `else` part of the statement
is executed.

A `while` statement is very similar to an `if` statement, except that
after each execution of the body, the condition is checked again, and the
loop continues until the condition evaluates as false.

Note that control-flow statements should evaluate to the value 0.
For example, the following program would produce the result 0:

```
if (1) {
  42;
}
```

### Functions and function calls

When a function definition is evaluated,

1. A function value should be created
2. The name of the function should be bound (assigned) to the function
   name in the global environment

A function value is created by creating a `Function` object, and making
it the dynamic representation of a `Value`. This could look something
like the following:

```c++
std::string fn_name;
std::vector<std::string> param_names;
Node *body;

// ...code to initialize fn_name, param_names, and body...

Value fn_val(new Function(fn_name, param_names, env, body));
```

In the code above, `env` is a pointer to the environment in which
the function is created, which for a top-level function will be the
global environment.

When a function call is evaluated,
the name of the called function is looked up in the current
environment (and recursively in parent environments if
necessary). If the value resulting from the lookup is not
a function value (or an intrinsic function value) an
`EvaluationError` should be raised.

If the called function value represents an ordinary "user-defined"
function (as opposed to an intrinsic function), the number
of arguments should be compared to the called function's number
of parameters. If they are different, an `EvaluationError`
should be raised.

If the called function value represents an intrinsic function,
then the arguments should be collected in an array, and then the intrinsic
function's implementation should be called
using its function pointer.  Assuming `fn_val` is a `Value`
representing the intrinsic function, and `loc` is a `Location`
indicating the location of the function call in the source
code, this will look something like this:

```c++
Value args[num_args];
// store arguments in the args array

IntrinsicFn fn = fn_val.get_intrinsic_fn();
Value result = fn(args, num_args, loc, this);
```

(We are assuming that this code is in a member function of
`Interpreter`, so `this` refers to the `Interpreter` object.)

If the called function value represents an ordinary "user-defined"
function, the procedure is as follows:

1. A function call environment whose parent is the function's
   parent environment should be created
2. Each argument should be evaluated in the current environment
   and bound (assigned) to the corresponding function parameter
3. The body of the function is recursively evaluated in the
   function call environment; the resulting `Value` is the
   result of the function call

Note that in the same way that the value of the last statement in
the overall Unit is the result of the program, the value of
the last statement in a function body is the result of the function.

### Blocks and scopes

As with [Assignment 1](assign01.html), the global environment is the
scope corresponding to the top-level Unit node in the overall AST.

Because we want our interpreter language to be a block-structured
language, every construct which surrounds an occurrence of SList
in curly braces ("`{`" and "`}`") defines a nested scope.

So, when the interpreter evaluates an SList (statement list),
it should create a nested environment (whose parent is the current
environment) for evaluating the code in that statement list.

It is not legal for a name to be defined more than once in any
block. However, it is legal for a nested block to define a name
that is defined in an outer block. This is why a function such as

```
function f(a, b) {
  var b;
  b = 3;
  a * b;
}
```

would be legal. The variable `b` defined in the body of the function
is a different variable than the parameter `b` defined in the function
call environment (which is the parent environment of the function
body environment.)

### Intrinsic functions

*Coming soon!*

### Values and reference counting of dynamic representations

*Coming soon!*

### Intrinsic functions

*Coming soon!*

### Arrays and array intrinsic functions

*Coming soon!*

### Strings and string intrinsic functions

*Coming soon!*

### Lambdas (anonymous functions)

*Coming soon!*

## Submitting

*Coming soon!*
