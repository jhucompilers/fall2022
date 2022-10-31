---
layout: default
title: "Assignment 4"
---

*This is a somewhat preliminary assignment description, primarily intended
to allow you to make progress on Milestone 1. It will be updated.*

*Update 10/30*: There are a few things to note regarding Milestone 1:

* You should add `local_storage_allocation.cpp` to the definition of the
  `SRCS` macro in the `Makefile`
* To support the address-of and pointer dereference operators, as well
  as the unary `!` and `-` operators, your `HighLevelCodeGen` class will
  need to override `visit_unary_expression`
* To support references to fields of struct instances, your `HighLevelCodeGen`
  class will need to override `visit_field_ref_expression` and
  `visit_indirect_field_ref_expression`

Also, you will need to add a `get_global_symtab` member function to the
`SemanticAnalysis` class. This could be defined in the header file as

```c++
SymbolTable *get_global_symtab() { return m_global_symtab; }
```

Also, the [Example high-level translations](#example-high-level-translations)
section has been updated with some example programs which use pointers
and instances of struct types.

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
is stored, so that it can generation instructions which use these values,
and also handle assignments which store values to these variables.

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
  opcode, and between 0 and 3 operands
* an `InstructionSequence` is a sequence of `Instruction` objects,
  with support for defining labels that can be used as control-flow
  targets

The starter code for the `HighLevelCodeGen` class also uses these classes,
and that code should serve as a useful guide.

The high-level opcodes are defined in the files `highlevel.h` and
`highlevel.cpp`, which are generated by a Ruby script called
`gen_highlevel_ir.rb`. If you wanted to, you could modify this
script to define additional high level instructions, or even change
the set of high-level opcodes available. However, the opcodes provided
are meant to be sufficient for the subset of C we will be targeting.

### High-level opcodes

The following table summarizes the intended meaning of the various
high-level opcodes, as generated by the `gen_highlevel_ir.rb` script
provided in the starter code.  Note that *\_sz* is an operand size suffix,
which will be one of the following:

* `_b`: "byte", 1 byte (8 bits)
* `_w`: "word", 2 byte (16 bits)
* `_l`: "long", 4 byte (32 bits)
* `_q`: "quad", 8 byte (64 bits)

Also note that for the `uconv` and `sconv` instructions, there are
two operand size suffixes, describing a promotion from a less precise
integer type to a more precise integer type. For example, the opcode
`sconv_bl` would convert an 8-byte signed value to a 32-bit signed
value.

Because the target architecture (x86-64) uses 64 bit pointers, you should
consider all pointers to be 8 bytes (64 bits.)

Opcode              | Operands      | Meaning
------------------- | ------------- | -------
`HINS_nop`          |               | does nothing
`HINS_add`*_sz*     | dst, src<sub>l</sub>, src<sub>r</sub> | add two integer values, store result in dst
`HINS_sub`*_sz*     | dst, src<sub>l</sub>, src<sub>r</sub> | subtract src<sub>r</sub> from src<sub>l</sub>, store result in dst
`HINS_mul`*_sz*     | dst, src<sub>l</sub>, src<sub>r</sub> | multiply two integer values, store result in dst
`HINS_div`*_sz*     | dst, src<sub>l</sub>, src<sub>r</sub> | divide src<sub>l</sub> by src<sub>r</sub>, store quotient in dest
`HINS_mod`*_sz*     | dst, src<sub>l</sub>, src<sub>r</sub> | divide src<sub>l</sub> by src<sub>r</sub>, store remainder in dest
`HINS_cmplt`*_sz*   | dst, src<sub>l</sub>, src<sub>r</sub> | determine if src<sub>l</sub> &lt; src<sub>r</sub>, set dst to 0 or 1
`HINS_cmplte`*_sz*  | dst, src<sub>l</sub>, src<sub>r</sub> | determine if src<sub>l</sub> &le; src<sub>r</sub>, set dst to 0 or 1
`HINS_cmpgt`*_sz*   | dst, src<sub>l</sub>, src<sub>r</sub> | determine if src<sub>l</sub> &gt; src<sub>r</sub>, set dst to 0 or 1
`HINS_cmpgte`*_sz*  | dst, src<sub>l</sub>, src<sub>r</sub> | determine if src<sub>l</sub> &ge; src<sub>r</sub>, set dst to 0 or 1
`HINS_cmpeq`*_sz*   | dst, src<sub>l</sub>, src<sub>r</sub> | determine if src<sub>l</sub> = src<sub>r</sub>, set dst to 0 or 1
`HINS_cmpneq`*_sz*  | dst, src<sub>l</sub>, src<sub>r</sub> | determine if src<sub>l</sub> &ne; src<sub>r</sub>, set dst to 0 or 1
`HINS_neg`*_sz*     | dst, src                              | store the negation of src in dst
`HINS_not`*_sz*     | dst, src                              | store the logical negation of src in dst
`HINS_mov`*_sz*     | dst, src                              | store src in dst
`HINS_sconv_bw`     | dst, src                              | promote 8 bit src to 16 bit dest (signed)
`HINS_sconv_bl`     | dst, src                              | promote 8 bit src to 32 bit dest (signed)
`HINS_sconv_bq`     | dst, src                              | promote 8 bit src to 64 bit dest (signed)
`HINS_sconv_wl`     | dst, src                              | promote 16 bit src to 32 bit dest (signed)
`HINS_sconv_wq`     | dst, src                              | promote 16 bit src to 64 bit dest (signed)
`HINS_sconv_lq`     | dst, src                              | promote 32 bit src to 64 bit dest (signed)
`HINS_uconv_bw`     | dst, src                              | promote 8 bit src to 16 bit dest (unsigned)
`HINS_uconv_bl`     | dst, src                              | promote 8 bit src to 32 bit dest (unsigned)
`HINS_uconv_bq`     | dst, src                              | promote 8 bit src to 64 bit dest (unsigned)
`HINS_uconv_wl`     | dst, src                              | promote 16 bit src to 32 bit dest (unsigned)
`HINS_uconv_wq`     | dst, src                              | promote 16 bit src to 64 bit dest (unsigned)
`HINS_uconv_lq`     | dst, src                              | promote 32 bit src to 64 bit dest (unsigned)
`HINS_ret`          | *none*                                | return from function
`HINS_jmp`          | label                                 | unconditional jump to label
`HINS_call`         | label                                 | call to function named by label
`HINS_enter`        | immediate                             | reserve specified number of bytes for local variables
`HINS_leave`        | immediate                             | deallocate specified number of bytes for local variables
`HINS_localaddr`    | dst, immediate                        | store pointer to local variable at given offset in dst
`HINS_cjmp_t`       | dst, label                            | conditional jump to label if dst contains true val
`HINS_cjmp_f`       | dst, label                            | conditional jump to label if dst contains false val

A few things to note:

* The `HINS_cmp`*xx* instructions compare two integer values, and set
  the destination register to a boolean value (0 or 1) depending on
  whether the condition tested was false or true
* The `HINS_cjmp_t` and `HINS_cjmp_f` use a boolean value produced by
  a comparison to conditionally jump to a target label if the
  condition is true (`cjmp_t`) or false (`cjmp_f`)
* The `HINS_sconv_`*xx* and `HINS_uconv_`*xx* instructions are
  intended to be used for promotions of integer values to a more
  precise (i.e., larger) type

Also note that because each two-operand arithmetic instruction
has 4 variants for the various operand sizes, the following function
is provided:

```c++
HighLevelOpcode get_opcode(HighLevelOpcode base_opcode, const std::shared_ptr<Type> &type) {
  if (type->is_basic())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(type->get_basic_type_kind()));
  else if (type->is_pointer())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(BasicTypeKind::LONG));
  else
    RuntimeError::raise("attempt to use type '%s' as data in opcode selection",
                        type->as_str().c_str());
}
```

The intent of this function is that you can pass it the "`_b`" variant
of an opcode (the "base" opcode), and the data type upon which you want
the instruction to operate, and it will return the correct variant
of the opcode for that type.

### Example high-level translations

Here are some example translations of test programs in the public test repository
to high-level code:

Example program | Example translation
:-------------: | :-----------------:
[example01.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example01.c) | [example01.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example01.txt)
[example02.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example02.c) | [example02.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example02.txt)
[example03.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example03.c) | [example03.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example03.txt)
[example04.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example04.c) | [example04.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example04.txt)
[example05.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example05.c) | [example05.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example05.txt)
[example06.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example06.c) | [example06.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example06.txt)
[example07.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example07.c) | [example07.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example07.txt)
[example08.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example08.c) | [example08.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example08.txt)
[example09.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example09.c) | [example09.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example09.txt)
[example10.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example10.c) | [example10.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example10.txt)
[example11.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example11.c) | [example11.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example11.txt)
[example12.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example12.c) | [example12.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example12.txt)
[example13.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example13.c) | [example13.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example13.txt)

These example translations are by no means the only reasonable translations
possible for these programs, but they should serve as a source of ideas
for your high-level code generator.

### Hints and tips

*It is possible that this section will be updated.*

*Allocating storage*: As mentioned previously, the `StorageCalculator` class
can be used to lay out the fields of a struct data type, but it can also
be used to lay out local variables in a block of memory in the stack frame.

The reference implementation does use `StorageCalculator`, and also implements
an optimization: specifically, variables which have non-overlapping lifetimes
can share storage. For example, in the code

```c
if (a) {
  int b1[1];
  b1[0] = 1;
  c = a + b1[0];
} else {
  int b2[1];
  b2[0] = 2;
  c = a + b2[0];
}
```

the arrays `b1` and `b2` cannot exist at the same time, and can use the same
storage. You are not required to implement this optimization, but if you are
interested, you can make use of the property that a `StorageCalculator` object
has value semantics. So, if `m_storage_calc` is a `StorageCalculator` member variable
representing the memory storage for variables that are currently in scope,
then the following code could be used to handle variable definitions in a
nested scope:

```c
// enter nested scope
StorageCalculator save = m_storage_calc;

// ...handle variable declarations in nested scope, allocate their
//    storage using m_storage_calc...

// leave nested scope
m_storage_calc = save;
```

The idea is that when control leaves a nested scope, memory storage allocated
for any variables in that scope will no longer be needed.

In general, you will need to make sure that the local memory area in the
stack frame (as specified in the `enter` and `leave` instructions) is large
enough to accommodate all of the local variables requiring memory storage.

*Storing operands in expression nodes.* One of the most important things
the high-level code generator will need to know is, for variables, array elements,
struct fields, and computeed values, the location serving as storage
for that variable or value.  A storage location is one of the following
possibilities:

* a global variable accessed by name
* a local variable or temporary value whose storage is a virtual register
* a local variable or temporary value whose storage is memory at a
  specific offset in the stack frame's local variable area

As the code generator recursively generates high-level code for expressions,
it should store an `Operand` in the node indicating the storage location
associated with that node.

For computed values (for example, the result of an addition), the code
generator should allocate a temporary virtual register, and use that
virtual register as the storage location. So, the Operand could be
constructed like this:

```c++
int temp_vreg = /* allocate the next unused temporary vreg number */;
Operand operand(Operand::VREG, temp_vreg);
```

Any expression node that is an lvalue should be annotated with an operand
that exactly represents the storage location if the variable, array element,
or struct field that the lvalue represents. In the case of local variables
whose storage is a virtual register, this is easy: the operand should just
name that virtual register. For values located in memory, the operand
should be a memory reference via an address stored in a virtual register.
It is very possible that you will need to generate a sequence of instructions,
possibly involving temporary virtual registers, to compute the exact address
of the referenced lvalue.  You can see this happening in
[example09.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example09.txt),
because each array element reference requires an address computation.

*Function calls.* The model for function calls is as follows:

* the `vr0` register is the return value register, so whatever value
  is in that register when a function returns is its return value
* the `vr1` through `vr9` registers are the argument registers: the
  first argument is passed in `vr1`, the second is passed in `vr2`,
  etc.

To call a function, generate code for each argument expression, and
move its value into the appropriate argument register. Then emit a
`call` instruction with the name of the function as the label operand.
(We will not be supporting function calls by any mechanism other than
directly naming the called function.) When the called function returns,
the return value will be in `vr0`.

One important detail is how a function should manage its own arguments.
It's fairly straightforward.

1. Parameters should have storage allocated just like any other local variable.
   Their storage is *not* the argument register containing the parameter value.
2. At the beginning of the generated high-level code for a function, the
   value of each argument register should be copied into the storage location
   of the corresponding parameter variable.
3. When the code generator needs to emit a function call, it can simply
   place the argument values in the appropriate argument registers.

## Milestone 2: x86-64 code generation

*Information coming soon!*

TODO: new source files needed, new Makefile

TODO: updated operand.h/operand.cpp (needed for `Operand::MREG64_MEM_OFF`
operands to work correctly)

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
zip -9r solution.zip *.h *.cpp *.rb *.y *.l Makefile README.txt
```

Upload the zipfile to Gradescope as **Assignment 4**.
