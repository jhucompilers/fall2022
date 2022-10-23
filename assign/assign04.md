---
layout: default
title: "Assignment 4"
---

Describe the IR classes. (See what we can re-use from last time.)

High level code generation: store an `Operand` in the `Node` to represent
the place where its value is stored.  So, to generate code for an AST
node:

1. Generate instructions that will place the result of the evaluation
   in a vreg or in memory
2. Set the node's operand to reflect the place where the value was
   stored

Strategy: start with very simple test programs that use very few
language features. Use assertions to make the compiler exit if it
reaches a language feature that's not handled yet.

(Maybe suggest an order of test programs to try.)
