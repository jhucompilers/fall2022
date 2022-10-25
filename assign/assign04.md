---
layout: default
title: "Assignment 4"
---

*This is a somewhat preliminary assignment description, primarily intended
to allow you to make progress on Milestone 1. It will be updated.*

**Due dates**:

Milestone 1: due Friday, Nov 4th by 11pm

Milestone 2: due Friday, Nov 18th by 11pm

Note that this assignment is a "double" assignment, and is worth twice
as much as Assignments 1, 2, or 3.

## Overview

In this assignment, you will implement code generation, turning
the semantic analyzer you implemented in [Assignment 3](assign/assign03.html)
into an actual C compiler capable of compiling programs into
x86-64 assembly language.

Note that the expectation for this assignment is generation of
*functioning* code, but not generation of *efficient* code.
In assignment 5 (the final assignment), you will implement code
optimizations to improve the efficiency of the "baseline" code
your compiler generates in this assignment.

## Getting started

I would recommend starting by making a copy of your code for assignment 3.

Download [assign04.zip](assign04.zip). This zipfile contains additional
source and header files, a Ruby script called `gen_highlevel_ir.rb`,
and an updated `Makefile`. You should incorporate these new and updated
files into your code base.  If you have made any changes to files that
are modified relative to Assignment 3, you will need to merge your
changes into the new versions.

## Grading breakdown

Your assignment grade will be determined as follows.

* Milestone 1
  * generating high-level IR code for selected test programs: 10%
* Milestone 2
  * generating x86-64 code for selected test programs: 80%
  * design and coding style: 10%

## Milestone 1: high-level code generation

Your first task for this assignment is to translate input C source code
into a "high-level" code-like intermediate representation, quite similar
to the <span style="font-variant: small-caps;">Iloc</span> intermediate
representation described in the textbook.

The goal of high-level code generation is to translate the input program
(in the annotated-AST form produced by your semantic analyzer)
into a form that precisely encodes

* which global variables, functions, and local variables exist, and
* which operations are executed when the implemented functions
  are called

As an example: consider the following C code:

```c
int main(void) {
  int n, i, sum;

  n = 11;
  i = 1;
  sum = 0;

  while (i <= n) {
    sum = sum + i;
    i = i + 1;
  }

  return sum;
}
```

This code could be translated into the following high-level IR code:

```
	.section .text

	.globl main
main:
	enter    $0
	mov_l    vr13, $11
	mov_l    vr10, vr13
	mov_l    vr13, $1
	mov_l    vr11, vr13
	mov_l    vr13, $0
	mov_l    vr12, vr13
	jmp      .L1
.L0:
	add_l    vr13, vr12, vr11
	mov_l    vr12, vr13
	mov_l    vr13, $1
	add_l    vr14, vr11, vr13
	mov_l    vr11, vr14
.L1:
	cmplte_l vr15, vr11, vr10
	cjmp_t   vr15, .L0
	mov_l    vr0, vr12
	jmp      .Lmain_return
.Lmain_return:
	leave    $0
	ret
```

A few things to note, to help explain the meaning of the high-level
code:

* Operands with names beginning with `vr` are "virtual registers";
  there is no limit to the number of virtual registers that a function
  can use
* `vr0` is the return value, `vr1` through `vr9` are argument
  registers, `vr10` and above are used for local variables and
  temporary values
* In high-level instructions, the first operand is the destination
* The suffix `_l` means that the operands are 32 bits in size
* Comparision instructions such as `cmplte_l` compare the second
  and third operand values, and store a boolean value (false or true)
  in the destination operand
* `enter` and `leave` set up and tear down a stack frame, and
  indicate the number of bytes of memory storage required for local
  variables

In the translation above, the virtual registers `vr10`, `vr11`. and
`vr12` are allocated as storage for the `int` local variables
`n`, `i`, and `sum`, respectively.  In general, if a variable's
type is either integral or a pointer, and its address is not taken,
its storage can be a virtual register.  Arrays and structs will require
storage in memory.

### Ok, how do I get started on this?

To generate high-level IR code, you will need to do two things:

1. Implement the `LocalStorageAllocation` visitor class so that
   it can determine a storage location (memory or virtual register)
   for each parameter and local variable in a function
2. Implement the `HighLevelCodeGen` visitor class so that it can
   construct an `InstructionSequence` containing high-level
   `Instruction`s which completely describe the storage requirements
   and dynamic operations of a function

The idea with `LocalStorageAllocation` is fairly simple:

* If a parameter or local variable can have its storage allocated
  as a virtual register (because it is either integral or a pointer,
  and its address is never taken), it should be assigned a specific
  virtual register as its storage. This virtual register should
  not be used for any other purpose in the function.
* If a parameter or local variable requires storage in memory,
  because either its address is taken, or it is an array or struct,
  it will need to be allocated an offset in the function's local storage
  area.   The `StorageCalculator` class (in `storage.h` and `storage.cpp`)
  is useful for this purpose, but you are welcome to come up with your
  own way of determining a storage offset for each variable requiring
  memory storage.

You will notice that in `context.cpp`, in the `Context::highlevel_codegen`
function, there is code which creates a `LocalStorageAllocation`
visitor and applies it to a function before high-level code generation.
Its goal should be to assign either a virtual register or a storage offset
to every local variable.  It will make sense to store this information
in the symbol table entries (i.e., `Symbol` objects) for each variable.
The idea here is that prior to high-level code generation, your compiler
will know precisely where the value of each parameter and local variable
is stored, so that it can generation instructions which use these values.

Once you have local storage allocation working, you can move on to
`HighLevelCodeGen`. Its goal is to append `Instruction` objects to an
`InstructionSequence` representing the high-level instructions of a
compiled function.

You can test high-level code generation using the `-h` command line
option. For example, if `$ASSIGN04_DIR` is the directory containing your
`nearly_cc` executable, then the command

```
$ASSIGN04_DIR/nearly_cc -h input/example02.c
```

would print a textual representation of the high-level code generated for
the input program `input/example02.c`.

### `Operand`, `Instruction`, and `InstructionSequence`

The `Operand`, `Instruction`, and `InstructionSequence` classes are used
to for both high-level and low-level "code-like" (a.k.a. "linear")
intermediate representations of functions.

I recommend reading through the code of these classes, since that
is likely the best way to learn how they work.  As a very brief summary:

* an `Operand` represents a virtual register, a memory reference
  via a base register (and optionally an index or offset),
  an intermediate value, or a label
* an `Instruction` represents a high-level instruction with an
  opcode, and between 1 and 3 operands
* an `InstructionSequence` is a sequence of `Instruction` objects,
  with support for defining labels that can be used as control-flow
  targets

The high-level opcodes are defined in the files `highlevel.h` and
`highlevel.cpp`, which are generated by a Ruby script called
`gen_highlevel_ir.rb`.

### High-level opcodes

The following table summarizes the intended meaning of the various
high-level opcodes.  Note that *\_sz* is an operand size suffix,
which will be one of the following:

* `_b`: "byte", 1 byte
* `_w`: "word", 2 byte
* `_l`: "long", 4 byte
* `_q`: "quad", 8 byte

Because the target architecture (x86-64) uses 64 bit pointers, you should
consider all pointers to be 8 bytes (64 bits.)

<!--
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
-->
