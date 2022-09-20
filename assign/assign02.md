---
layout: default
title: "Assignment 2"
---

**Due dates**:

* Milestone 1 due **Mon, Sept 26** by 11pm Baltimore time
* Milestone 2 due **Mon, Oct 3** by 11pm Baltimore time

*Update 9/18*: You will need updated versions of several starter code files;
see the [Updated files](#updated-files) section.

*Update 9/20*: Clarified expectations for Milestone 1: see [Milestones](#milestones).

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

### Updated files

There are some updated starter files that you will need. These
correct some bugs in the original starter code. You can download
them into your working directory by running the following commands:

```
wget https://jhucompilers.github.io/fall2022/assign/assign02/function.cpp
wget https://jhucompilers.github.io/fall2022/assign/assign02/valrep.h
wget https://jhucompilers.github.io/fall2022/assign/assign02/valrep.cpp
```

Here are direct download links:

* [function.cpp](assign02/function.cpp)
* [valrep.h](assign02/valrep.h)
* [valrep.cpp](assign02/valrep.cpp)

You should not have needed to modify any of these files for
Assignment 1, so copying the new versions should not cause any issues.
If you have made changes to these files, you should use `diff` to find
the textual differences between your version and the downloaded version.

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

Note that strings and lambdas represent only 3% of the grade for the assignment,
so it is completely reasonable for you to treat these parts of the
assignemnt as optional.

### Milestones

There are two milestones:

* Milestone 1: control flow and calls to intrinsic `print`/`println`
  work correctly in programs using integer values
* Milestone 2: all functionality is implemented

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

Binding the name of the function to its value is just like
a variable assignment, and in fact, in our language functions
are just variables whose initial value is a user-defined function.

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

An *intrinsic function* is one that is built into the interpreter,
rather than being defined by the interpreted program.

There are many ways an interpreter could implement intrinsic functions.
The one suggested by the starter code is to simply implement
them as C++ functions in the interpreter program. For example,
here possible implementation of the required `print` function:

```c++
Value Interpreter::intrinsic_print(Value args[], unsigned num_args,
                                   const Location &loc, Interpreter *interp) {
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
  printf("%s", args[0].as_str().c_str());
  return Value();
}
```

You will probably want your implementations of intrinsic functions to be `static`
member functions. They definitely should not be non-static member functions.

The names of the intrinsic functions should be bound to their implementations
in the global environment before the interpreted program is executed.
This could look something like the following:

```c++
global_env->bind("print", Value(&intrinsic_print));
```

This assumes that `global_env` is a pointer to the global environment
object, and that your `Environment` class has a member function called
`bind` which creates a variable and immediately associates it with
a `Value`.

You will need to implement the following intrinsic functions.

`print`: prints the textual representation of a single value to standard
output. A possible implementation of this function is shown above.
It should raise an `EvaluationError` if it is not passed exactly
one argument. The `Value::as_str` member function should be used
to get the string value to print.

`println`: like `print`, but prints a newline ("`\n"`) character
after the textual representation of the printed value.

`readint`: Reads a single integer value from standard input and
returns it. It should not be passed any arguments, and should raise
`EvaluationError` if it is passed any arguments. You should use
`scanf` to read the integer input value.

### Values and reference counting of dynamic representations

Value representing integers or intrinsic functions are considered
*atomic*, meaning that the representation of the value requires
a fixed number of bytes, which are stored directly in the
`Value` object and can be copied by value.

The other types of values the interpreter can support, specifically

* functions
* arrays
* strings
* environments (if implementing closures, otherwise environments aren't values)

require an unknown amount of storage. These kinds of values require
a *dynamic* representation. Each dynammic representation should be implemented
as a class inheriting from the `ValRep` base class.

The starter code already implements a `Function` class to represent user-defined
functions.

You will need to add `Array` and `String` classes to implement the dynamic
representations for array and string values.

Because `Value` objects representing types requiring a dynamic representation
are implemented using a pointer to a dynamically-allocated instance of
`ValRep` (`Function`, `Array`, `String`, etc.), a problem arises. Specifically,

1. more than one `Value` object can have a pointer to the same dynamic
   representation object, and
2. we want to make sure that dynamic representation objects that are no
   longer needed are deallocated

You should use *reference counting* to manage dynamic representation objects.
The idea is that

1. Each time a `Value` object acquires a pointer to a dynamic
   representation object, it should increment the reference count of the
   object
2. Each time a `Value` object releases a pointer to a dynamic
   representation object, it should decrement the reference count
   of the object
3. If, when a `Value` releases a pointer to a dynamic representation
   object and decrements its reference count, it notices that the
   object's reference count becomes 0, it should delete the object

A reference count member variable and accessor functions
(`add_ref`, `remove_ref`, `get_num_refs`) are already implemented in `ValRep`.
You will need to implement reference counting in the `Value` class,
including deleting dynamic representation objects that no longer have
any references.

Note that your interpreter should treat `Value` as having value semantics,
meaning that rather than passing, returning, or storing pointers or references
to `Value` objects, your interpreter should pass them by value, return
them by value, copy them by value, etc.

### Arrays and array intrinsic functions

Note: implementing arrays and the array intrinsic functions is only worth
4% of the assignment grade.  You should only add support for arrays after
you have completely implemented control flow, functions, and the
required `print`, `println`, and `readint` intrinsic functions.

It is useful for a programming language to support some kind of
collection data structure. Arrays or vectors are one very useful
collection data type.

Support for arrays can be added by introducing ararys as a new kind
of dynamic value (supported by an `Array` class inheriting from `ValRep`),
and the following intrinsic functions:

Name   | Num args | Arg types | Description
------ | -------- | --------- | -----------
`mkarr`| varies   | varies        | returns an array value in which each argument value is an element value of the array
`len`  | 1        | array         | when passed a single array value, returns an integer value indicating the number of elements in the array value
`get`  | 2        | array, integer      | returns the value of the element at the given integer index in the specified array
`set`  | 3        | array, integer, any | sets element at specified index to the value specified by the third argument; returns the stored value
`push` | 2        | array, any          | appends specified value (second argument) to the end of the specified array, increasing its length by 1
`pop`  | 1        | array               | removes and returns last element of array, decreasing its length by 1; raises `EvaluationError` if the array is empty

If an array intrinsic function is passed the wrong number and/or types of arguments, it should raise an `EvaluationError`.

Note that you will need to modify the `Value::as_str` member function so that it
can handle array values. An array value should be converted to a string
as follows:

* the first character of the string is `[`
* the result of converting each element value to a string is appended,
  with each adjacent pair of elements being separated by a space
  followed by a comma
* the last character of the string is `]`

### Strings and string intrinsic functions

Note that implementing support for strings and the intrinsic string functions
is only 2% of the assignment grade, so we recommend not working on this
until control flow, functions, and arrays are completely tested and working.

Support for strings is another useful data type.

You will need to start by adding support for string literals to the lexer.
A string literal is a sequence of characters

* starting with `"`
* continuing with 0 or more occurrences of (in any order)
  * a character other than `\` or `"`
  * one of the escape sequences `\"`, `\n`, `\r`, or `\t`
* ending with `"`

String literals should be allowed as an expansion of the F nonterminal symbol,
so you will need to add the production

```
F → string_literal
```

to the parser.

When a string literal is converted to a value, the sequence of characters
represented by the resulting string is determined by the characters in the
string's lexeme, excluding the quote ("`"`") characters at the beginning
and end of the lexeme. Also note that the escape sequences
`\"`, `\n`, `\r`, and `\t` stand for (respectively) the quote
character (`"`), newline, carriage return, and tab.

A string value requires a dynamic representation, so you will need to add
a `String ` class deriving from `ValRep` to store the sequence of characters
represented by the string.

The following intrinsic functions are defined to work with string values:

Name     | Num args | Arg types        | Description
-------- | -------- | ---------------- | -----------
`substr` | 3        | string, int, int | returns a substring consisting of all characters starting at the beginning index (second arg) and containing as many subsequent characters as indicated by the third arg; returns an empty string if the substring doesn't fall entirely in the bounds of the string value
`strcat` | 2        | string, string   | returns the concatenation of two string values
`strlen` | 1        | string           | returns the length (number of characters) in a string value

If a string value is called with the wrong number and/or types of arguments,
an `EvaluationError` error should be raised.

When converting a string value to a string using `Value::as_str`, the resulting
string should simply reflect the sequence of characters in the string value.

### Lambdas (anonymous functions)

Note: support for lambdas is only 1% of the assignment grade. We
**strongly** recommend that you if you choose to implement lambdas,
that you don't work on them until all other features are completely
working and tested.  Even if you do decide to work on lambdas, be aware
that supporting them will be a very intrusive change to the interpreter,
and you should probably work on a branch, or take other steps to allow
you to easily revert your code back to its original pre-lambda state.

A *lambda*, a.k.a. *closure*, is an anonymous function. Many languages, including
JavaScript, C++, Java, Python, Ruby, and more or less every functional languages,
support lambdas in some form.

Lambdas should be supported by adding the following rule to your parser:

```
F → function ( OptPList ) { SList }
```

In principle, supporting lambdas is very simply. They are simply functions
created "on-the-fly" as values, with the current evaluation environment
as their parent environment.

The tricky aspect of lambdas is that the evaluation environment is typically
the environment created to evaluate a call to the function that created the
lambda value. For example:

```
function mkadd(n) {
  function(x) { x + n; };
}

var add1;
add1 = mkadd(1);
add1(2);
```

The result of this program is 3. The `mkadd` function returns a lambda
(anonymous function) which returns the sum of

* its parameter `x`, and
* the value of the parameter `n` of the call to `mkadd`
  which created the lambda

The tricky aspect of lambdas is that they must have access to variables
defined in the environment in which they were created. In the case of
an environment associated with a function call, the problem is that
the lambda value will "outlive" the duration of the function which
created it, meaning that retaining a pointer to the environment(s)
created for the evaluation of that function will result in the lambda
containing a pointer to an environment which no longer exists.

There are various solutions to this problem. One would be to have
the lambda copy the values of variables it will use.  This would require
the interpreter to analyze the body of each lambda to determine which
"free" variables it uses, and where they will be defined when
the lambda is created.

The solution we will suggest is to make `Environment` a reference counted
dynamic representation type, similar to `Function`, `Array`, `String`,
etc. The interpreted program will not actually use environment values
directly. However, anywhere the interpreter needs to use an environment,
it will use a `Value` as a smart pointer to ensure that the environment
object is properly reference counted. This means that everywhere
the interpreter was referring to a current environment object via
a pointer (`Environment *env`), it will now need to use a `Value`
to wrap the environment (`Value env`).  This also means that code
calling functions on the current environment via a pointer
(`env->lookup(var_name)`) will need to extract the pointer to the
environment object from the value wrapping it (`env.get_env()->lookup(var_name)`).

This change will also mean that rather than allocating an environment
as a local variable,

```c++
Environment block_env(env);
```

you will need to dynamically-allocate an environment object and wrap
it using a `Value`:

```c++
Value block_env(new Environment(env));
```

In general, reference counting will ensure that environments are
properly deallocated when they are no longer in use. However, recall that
a limitation of reference counting as a strategy for storage allocation
is that it has difficulty reclaiming dynamic objects which have reference
cycles. One likely reference cycle in our interpreter is between
top-level functions and the global environment. The global environment
will have references to the function objects representing user-defined
functions. However, each of those functions will have the global
environment as its parent environment, resulting in a reference cycle.
This particular reference cycle is easy to break if the interpreter
simply clears the global environment (by removing bindings for all
global variables and functions) when the execution of the interpreted
program is complete.

## Testing

*Note: the tests for Assignment 2 are currently somewhat limited, although
they are a reasonable starting point. We will be adding more tests
in the very near future.*

The [fall2022-tests](https://github.com/jhucompilers/fall2022-tests)
repository has tests you can use to check your implementation.

The procedure for running tests is exactly the same as in [Assignment 1](assign01.html#testing),
except that

* you should set the `ASSIGN02_DIR` environment variable rather than `ASSIGN01_DIR`, and
* the tests are in the `assign02` directory rather than the `assign01` directory

We *strongly* encourage you to make substantial use of the tests as
you implement the syntax and semantics of the language constructs in this
assignment, since they demonstrate the expected language semantics
in a way that makes it easy to determine whether or not your interpreter
implements them correctly or incorrectly.

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
as **Assignment 2 MS1** or **Assignment 2 MS2** (depending on which
milestone you are submitting.)
