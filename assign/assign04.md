---
layout: default
title: "Assignment 4"
---

**Due dates**:

Milestone 1: due Friday, Nov 4th by 11pm

Milestone 2: due Friday, Nov 18th by 11pm

Note that this assignment is a "double" assignment, and is worth twice
as much as Assignments 1, 2, or 3.

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

*Update 11/2*: Information on how to get started on
[Milestone 2](#milestone-2-x86-64-code-generation)
has been added.

*Update 11/4*: The [Milestone 2](#milestone-2-x86-64-code-generation) section
is now fairly complete, contains information about testing and function calls,
and recommends a development strategy for the low-level code generator.

*Update 11/10*: Added a few more code generation examples to the
[Milestone 2](#milestone-2-x86-64-code-generation) section, and also described a
few more "built-in" functions used in the test programs.

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
`sconv_bl` would convert an 8-bit signed value to a 32-bit signed
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

To get started on Milestone 2, download [assign04-ms2.zip](assign04-ms2.zip).
This zipfile contains new and updated code that you will need to do low-level
code generation.

Note that if you have changed any of the files that are being updated,
you will need to incorporate your changes into the new version of the
file. We recommend the following procedure:

* **Make sure all of your current code is committed and pushed.**
  If you do this, then there is no way you can lose any work.
* Unzip the new zipfile in a separate directory, then copy all of the
  files to your working directory
* For each file, run the command <code class='highlighter-rouge'>git diff <i>filename</i></code>
  to see what changes there are. If you see changes indicating that code you
  wrote was deleted, then you will need to manually incorporate your code
  back into the file.

It is likely that the only file you changed that is updated in the zipfile
is `context.cpp`: specifically, the code you added to execute local storage
allocation.  Make sure that you incorporate this code into the updated
`context.cpp`.

Once you have updated all of the necessary files, rebuild using

```
make clean
make depend
make -j
```

You should find that the existing functionality (generating high-level code)
still works.  Once you've verified that, you can `git add` and `git push`
your changes.

If for some reason you would like to go back to the original version
of a file (rather than adding and committing the changed version), you can
do so with the command

<div class='highlighter-rouge'><pre>
git checkout <i>filename</i>
</pre></div>

### Low-level code generation

Your task in Milestone 2 is to translate each high-level instruction produced
by your high-level code generator into a sequence of low-level (x86-64)
instructions.

The `LowLevelCodeGen` class implements this transformation. Specifically, the
`translate_instruction` member function will be called for each high-level
`Instruction` object, and should append one or more low-level `Instruction`
objects to the `ll_iseq` (low-level instruction sequence) object which carry
out the operation represented by the high-level instruction.

Low-level opcodes and register numbers are defined in `lowlevel.h`.

Note that there are four different `Operand::Kind` values for machine
registers. Specifically, `Operand::MREG8`, `Operand::MREG16`, `Operand::MREG32`,
and `Operand::MREG64`. The `select_mreg_kind` helper function is intended
to make it easy to select the machine operand kind for a particular operand size.
Also note that `highlevel.h` (automatically generated by the `gen_highlevel_ir.rb`
script) has been updated to have two functions useful for determining the
size of the source and destination operands of a high-level instruction.
These functions are `highlevel_opcode_get_source_operand_size` and
`highlevel_opcode_get_dest_operand_size`.

Many of the low-level instructions, like the high-level instructions,
have 4 variations for the different operand sizes. The `select_ll_opcode`
helper function is intended to select the appropriate variant of a
low-level opcode.

You will see that translations of the `HINS_enter`, `HINS_leave`, and
`HINS_ret` high-level opcodes are already provided.

To give you a sense of what code to implement a translation might look like,
here is a possible implementation of a transformation of the `HINS_mov_b`/`w`/`l`/`q`
opcodes:

```c++
  if (match_hl(HINS_mov_b, hl_opcode)) {
    int size = highlevel_opcode_get_source_operand_size(hl_opcode);

    LowLevelOpcode mov_opcode = select_ll_opcode(MINS_MOVB, size);

    Operand src_operand = get_ll_operand(hl_ins->get_operand(1), size, ll_iseq);
    Operand dest_operand = get_ll_operand(hl_ins->get_operand(0), size, ll_iseq);

    if (src_operand.is_memref() && dest_operand.is_memref()) {
      // move source operand into a temporary register
      Operand::Kind mreg_kind = select_mreg_kind(size);
      Operand r10(mreg_kind, MREG_R10);
      ll_iseq->append(new Instruction(mov_opcode, src_operand, r10));
      src_operand = r10;
    }

    ll_iseq->append(new Instruction(mov_opcode, src_operand, dest_operand));
    return;
  }
```

Note that the `get_ll_operand` ("get low-level operand") function returns an
low-level `Operand` naming the storage location for a given high-level
operand. It is passed a `std::shared_ptr` to the low-level
`InstructionSequence` because it might be necessary to generate one or more
low-level instructions in order to create a meaningful low-level operand.

Also note that the generated low-level code uses the `%r10` register
(or one of its sub-registers) to store a temporary value, to avoid the possibility
of emitting a `mov` instruction with two memory operands. The `%r10` and
`%r11` registers are ideal for being used as short-term temporaries in the
low-level code, since they are caller-saved, and are not used for arguments
or return value in function calls.

### Example low-level translations of test programs

The [assign04 directory in the test repository](https://github.com/jhucompilers/fall2022-tests/tree/main/assign04),
in addition to having example high-level code translations of the test programs,
has example low-level translations in the `example_lowlevel_code`
subdirectory.

Here are some example translations:

Example program | Example HL translation | Example LL translation
:-------------: | :--------------------: | :--------------------:
[example01.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example01.c) | [example01.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example01.txt) | [example01.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example01.S)
[example02.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example02.c) | [example02.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example02.txt) |  [example02.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example02.S)
[example05.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example05.c) | [example05.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example05.txt) |  [example05.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example05.S)
[example09.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example09.c) | [example09.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example09.txt) |  [example09.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example09.S)
[example10.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example10.c) | [example10.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example10.txt) |  [example10.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example10.S)
[example12.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example12.c) | [example12.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example12.txt) |  [example12.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example12.S)
[example16.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example16.c) | [example16.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example16.txt) |  [example16.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example16.S)
[example23.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example23.c) | [example23.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example23.txt) |  [example23.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example23.S)
[example25.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example25.c) | [example25.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example25.txt) |  [example25.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example25.S)
[example27.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example27.c) | [example27.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example27.txt) |  [example27.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_lowlevel_code/example27.S)

These low-level translations may be useful as a reference.
Note that the test repository has more tests programs and example
translations.

### Generating references to virtual registers

The high-level instructions will have many references to virtual registers.

`vr0` is the return value register, so any references to `vr0` should
be translated to refer to `%rax`, or a sub-register (e.g., `%eax`)
of the appropriate size.

`vr1` through `vr6` are the function argument registers, and these should
be translated to refer to `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, and
`%r9`, respectively. Again, use the appropriate sub-register (e.g.,
`%edi`) depending on the operand size.

You can assume that `vr7` through `vr9` will not be used, because the
test programs will not have any functions with more than 6 parameters.

Virtual registers `vr10` and above will be used as storage for local
variables, or as storage for temporary values. *These should be allocated
in memory.* So, the local storage area will need to be large enough for
both local variables allocated in memory, and the virtual registers used
for locals and temporaries. You can reserve 8 bytes of local storage for
each virtual register.

### Testing your low-level code generator

The [public test repository](https://github.com/jhucompilers/fall2022-tests)
is intended to help you validate your low-level code generator.

Start by making sure that your clone of the test repository is up to date by
running `git pull`, then change directory into the `assign04` subdirectory.

Set the `ASSIGN04_DIR` environment variable to wherever your compiler project
is located, e.g.

```
export ASSIGN04_DIR=~/git/assign04
```

The `build.rb` script is used to invoke your compiler on a C source file,
and then assemble and link the code into an executable. You can run
it using the command

<div class='highlighter-rouge'><pre>
./build.rb <i>testname</i>
</pre></div>

where *testname* is the stem of one of the test programs. For example, use
the command

```
./build.rb example01
```

to assemble and link the
[input/example01.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example01.c)
test program. The `build.rb` script will create a directory called `out`
(if it doesn't already exist), and use this directory to store the
generated assembly languge source code. For example, if you are compiling
the `example01` test, the file `out/example01.S` will contain the low-level
code generated by your compiler. If the executable is assembled and linked
successfully, it will also be in the `out` directory. For the `example01`
test, the executable will be called `out/example01`, so you could run the
executable using the command

```
./out/example01
```

As with the previous assignments, a script called `run_test.rb` runs
a test and indicates whether the test passed or failed. So,

```
./run_test.rb example01
```

would invoke your compiler on `input/example01.c`, assemble and link the
generated code, invoke the program (providing input if the test requires it),
and then check the program exit code and generated output against the
expected exit code and output. A successful test will look
something like this:

<div class='highlighter-rouge'><pre>
$ <b>./run_test.rb example01</b>
Generated code successfully assembled, output exe is out/example01
Test passed!
</pre></div>

If the executable does not exit with the correct exit code (i.e., if it does
not return the correct value as the return value from the `main` function),
you will see something like this:

<div class='highlighter-rouge'><pre>
$ <b>./run_test.rb example01</b>
Generated code successfully assembled, output exe is out/example01
Executable exited with exit code 0, expected 3
</pre></div>

If the executable does not produce the expected output, you will see
something like this:

<div class='highlighter-rouge'><pre>
$ <b>./run_test.rb example18</b>
Generated code successfully assembled, output exe is out/example18
10a11
> 0
Output did not match expected output
</pre></div>

When the output does not match the expected output, you will see output from
the `diff` program summarizing which parts of the output did not match.

### Function calls, "built-in" functions

Function calls should be fairly straightforward to handle, as long as the
high-level code generator follows the recommendations mentioned previously.
Each occurrence to the `vr0` result register should be translated to become an
occurrence to `%rax` or a sub-register, and each occurrence of the `vr1` through
`vr6` argument registers should become occurrences of the appropriate
x86-64 argument registers (`%rdi`, `%rsi`, etc.) or sub-registers.

`call` instructions in the high-level code should become `call` instructions
in the low-level code, with the operand being a label naming the called
function. In fact, it is sufficient to handle them in the
low-level code generator as follows:

```c++
  if (hl_opcode == HINS_call) {
    ll_iseq->append(new Instruction(MINS_CALL, hl_ins->get_operand(0)));
    return;
  }
```

The `build.rb` script generates some "helper" functions for test programs
to use to read input values and print output values.  These have the following
prototypes:

```c
void print_str(const char *s);
void print_i32(int n);
int read_i32(void);
void print_i64(int n);
int read_i64(void);
void print_nl(void);
void print_space(void);
```

As long as your compiler can handle function declarations correctly,
you will not need to do anything special to allow calls to these functions to work.

You can examine the
[code for the build.rb script](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/build.rb)
to see the assembly code for these functions.

### Other hints and tips

*Development strategy*. You should develop your low-level code generator
incrementally. Start with the simplest test program, which is `example01.c`,
and get it working. Then move on to more complicated ones. The test programs
are numbered in a way that *roughly* indicates an order from less complex
to more complex, although there are some exceptions. For example,
`example14` is a better place to start when working on arrays and array
subscript operations than `example09`.

*Think very carefully about how local memory storage is addressed.*
In this version of your compiler, most values will (ultimately) be stored in memory,
because virtual registers will have their storage allocated in memory.
You will need to allocate space in memory for the virtual registers, and make
sure that it does not overlap the memory used for local variables allocated
in memory (such as arrays and struct instances.)

*Debugging*. You can run `gdb` on an executable that is misbehaving. Step through
the generated instructions line by line. Inspect values in memory. For example,
if `-48(%rbp)` contains a 64-bit integer value, you could print it using
either of the following commands:

```
print *((long *)($rbp - 48))
print/x *((long *)($rbp - 48))
```

The first command would print the value in decimal, and the second in
hexadecimal.

*Address computations should be 64 bit.* Because pointers are 64 bits, all address
computations should be 64 bit computations. You will need to think about
how to ensure this. In a source program, it is quite common for an array
index to be a 32-bit `int`; for example,

```c
int n;
int arr[3];
n = 0;
arr[n + 1] = 42;
```

The reference implementation uses the strategy of generating code for an
index value using whatever type is appropriate based on the original source
code, and then promoting that value to 64 bits before it is used to compute
an address. You will see `movslq` instructions in some of the example low-level
outputs, where a 32-bit index value is promoted to 64 bits.

## `README.txt`

For both milestones, please submit a `README.txt` with your submission
which briefly discusses anything interesting or significant you want us
to know about your implementation.  If there are any features you weren't
able to get fully working, or if you implemented extra functionality,
this is a place you could document that.

## Submitting

Create a zipfile with all of your code, your `Makefile`, and your `README.txt`.
Example commands:

```
make clean
zip -9r solution.zip *.h *.cpp *.rb *.y *.l Makefile README.txt
```

Upload the zipfile to Gradescope as **Assignment 4 MS1** or **Assignment 4 MS2**,
whichever is appropriate.
