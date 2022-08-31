---
layout: default
title: "Design and coding style expectations"
---

# Design, coding style, and efficiency

Assignments are graded mostly on functional correctness.  However, a portion of each assignment
grade (typically 10%) will be allocated to design, coding style, and efficiency.

## Correctness

Obviously, you should strive to make your code *correct*, i.e., free from
bugs.

We expect your code to be free of memory errors, such as

* use of uninitialized variables
* invalid pointer dereferences
* out of bounds array accesses
* double frees
* memory leaks

We *strongly* recommend that you use `valgrind` to test your code
to check for memory errors. You should expect that we will be using
`valgrind` on your code as part of assignment grading.

You can significantly ease the burden of ensuring that dynamically-allocated
objects are deallocated by using "smart pointer" classes such
as [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr).

## Compiler flags

Make sure that at a minimum, you are using the `-Wall` compiler option
to enable the most important compiler warnings. We expect your code to
compile cleanly (no compiler warnings.)

## Coding style

"Coding style" refers to things like

* Code formatting/indentation
* Names for variables, functions, data types
* Comments
* General organization of source modules

There is no required style, but we do expect you to use a reasonably standard
coding style, and that you are consistent in your coding style.  Your main consideration
should be making your code as easy to understand as possible.

Do *not* mix spaces and tabs.

Regarding commenting: a good rule of thumb is

* each function and major data type should have a longer documentation comment
  explaining its purpose
* for function/method bodies, each important section of code should have
  a concise comment explaining its purpose

## Design

Compilers and interpreters tend to be complex.  "Design" encompasses the larger
issues of how your overall program is organized.

In your design, you should strive for modularity.  Usually, a module is
centered around a major data type (such as a struct type or class) and operations
that can be performed on that type (functions/methods.)  To the extent possible,
distinct modules should be cleanly separated.  Each module should have a well-factored
set of public operations, and should not expose internal implementation details.
We strongly recommend structuring your code using C++ classes, and encapsulate
all private implementation details (member variables, helper functions)
by making them `private`.

## Efficiency

Our efficiency expectations are that, in general, you will choose efficient data
structures and algorithms where possible.  We are much more interested in
asymptotic efficiency than constant factors.  The situations where we might deduct
points for inefficient code is when a suboptimal algorithm or data structure
is used in a context where a more efficient algorithm or data structure could have
been used without any significant reduction in simplicity or understandability.

We *highly* encourage the use of STL data types such as strings, vectors, and maps,
since when used correctly they result in code that is both clean and efficient.
