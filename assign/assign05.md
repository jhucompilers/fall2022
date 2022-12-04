---
layout: default
title: "Assignment 5"
---

**Due date**: Friday, Dec 9th by 11pm

*Update 12/1*: Added a [Testing](#testing) section, and also filled in the
[Generated code examples](#generated-code-examples) section.
Also, the [Analysis and experiments](#analysis-and-experiments)
and [Report](#report) sections have been updated to more accurately
document expectations.

*Update 12/3*: Updated [Generated code examples](#generated-code-examples)
and added a [Peephole optimization](#peephole-optimization) section.
This information could be useful if you are interested in implementing
transformations to improve the quality of the idioms used in the generated
low-level code.

*Update 12/4*: A few additional peephole instruction selection optimizations
have been enabled in code generation examples. Also,
`lowlevel_defuse.cpp` was updated so that the destination operand in `leaq`
and `popq` instructions is not erroneously treated as a use.

## Overview

In this assignment, you will implement optimizations to improve the target code quality
compared to the code generator you implemented in [Assignment 4](assign04.html).

## Getting started

You might want to start by making a copy of your working for for Assignment 4.
However, if you are using Git for version control, you could probably just
continue working using your existing code base.

Download [assign05.zip](assign05.zip). You will need to incorporate
the files into your project.  Make sure your work is committed to Git
and pushed before incorporating the new files.  *In theory* you wouldn't
have needed to change any of these files. However, we strongly recommend
using `git diff` to check whether any of your code is being overwritten
by the new file versions, and manually reincorporating your changes
before proceeding. (Same drill as Assignment 4.)

## Grading criteria

* Optimizations implemented: 40%
* Report:
    * General discussion of optimizations implemented: 25%
    * Analysis and experimental evaluation: 25%
* Design and coding style: 10%

## Implementing optimizations

The primary metric you should be optimizing for is the run-time efficiency of the generated assembly code.

Optimizing for (smaller) code size is also valid, but should be considered a secondary concern.

Two benchmark programs are provided (in the
[assign05/input](https://github.com/jhucompilers/fall2021-tests/tree/master/assign05/input)
subdirectory of the [test
repository](https://github.com/jhucompilers/fall2021-tests)):

* [example29.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example29.c)
  multiplies 500x500 matrices represented as 250,000 element 1-D arrays
* [example31.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example31.c)
  multiplies 500x500 matrices represented as 500x500 2-D arrays

You can benchmark your compiler as follows (user input in **bold**):

<div class="highlighter-rouge"><pre>
$ <b>./build.rb example29</b>
Generated code successfully assembled, output exe is out/example29
$ <b>mkdir -p actual&#95;output</b>
$ <b>time ./out/example29 &lt; data/example29.in &gt; actual&#95;output/example29.out </b>

real	0m1.625s
user	0m1.620s
sys	0m0.004s
$ <b>diff expected_output/example29.out actual_output/example29.out</b>
</pre></div>

Note that the `build.rb` command above would build the *unoptimized* version
of the test program. To build the *optimized* version, the command should
be

```
./build.rb -o example29
```

Your can substitute other test program names in place of `example29`.

If the `diff` command does not produce any output, then the program's output matched the expected output.

The `build.rb` puts the generated assembly language code in the `out`
directory, so in the above test case, the assembly code would be in
`out/example29.S`.

The **user** time in the output of the `time` command represents the
amount of time the test program spent using the CPU to execute code,
and so is a good measure of how long it took the compiled program to do
its computation.  Your code optimizations should aim to reduce user time
for the benchmark programs.

## Testing

The [assign05](https://github.com/jhucompilers/fall2022-tests/tree/main/assign05)
directory in the test repository has the same test programs as in
[Assignment 4](assign04.html). Similar to the other assignments, you will need
to set the `ASSIGN05_DIR` environment variable to the directory containing
your compiled compiler program. For Assignment 5, the `build.rb`,
`run_test.rb`, and `run_all.rb` scripts have been updated so that they
can take a `-o` command line argument to enable optimizations.
For example:

```bash
# build an optimized version of example28
./build.rb -o example28
# test the compiler on example28, enabling optimizations
./run_test.rb -o example28
# run all tests with optimizations enabled
./run_all.rb -o
```

We **highly** recommend running both `./run_all.rb` and `./run_all.rb -o`
every time you make a change to your compiler. This will help you ensure that
it continues to work correctly both with and without optimizations
enabled.

## Analysis and experiments

As you work on your optimizations, do experiments to see how effective
they are at improving the efficiency of the generated code.

**Ideally**, you should use either the `example29` or `example31`
programs as the basis for your benchmarking. However,
you could use any test program as a basis for your experiments,
as long as it allows you to demonstrate a substantial improvement
in the quality of the generated code.

As you work on optimizations, you could start with some very simple test
programs, and then work your way up to the benchmark programs (which
are relatively complex.)

## Report

One of the most important deliverables of this assignment is a written
report that describes what optimizations you implemented and how effective
they were.

For each optimization technique you implemented, your report should document

* how the quality of the generated code was improved (include
  representative snippets of code before and after the optimization,
  for relevant test programs)
* how the efficiency of the generated code improved (i.e., how much did
  performance of the benchmark programs improve)

Your report should also discuss what inefficiencies remain in the
generated code, and what optimization techniques might be helpful for
addressing them.

Please make a substantial effort to demonstrate improvements on
"realistic" programs.  As mentioned above, `example29` and
`example31` are good candidates because they perform a substantial
and realistic computation.

## Hints and suggestions

In addition to the hints and suggestions included below, there is a
[detailed advice document](assign05-advice.html) which has some fairly
detailed suggestions for how to approach this assignment.

### How/where to implement optimizations

You will find `TODO` comments in the `generate` member function of the
`LowLevelCodeGen` class (in `lowlevel_codegen.cpp`) which indicate where
the high-level and low-level instruction sequences could be transformed.

Here is an idea of what you might want the code of your
`LowLevelCodeGen::generate` member function to look like:

```c++
std::shared_ptr<InstructionSequence>
LowLevelCodeGen::generate(const std::shared_ptr<InstructionSequence> &hl_iseq) {
  Node *funcdef_ast = hl_iseq->get_funcdef_ast();

  // cur_hl_iseq is the "current" version of the high-level IR,
  // which could be a transformed version if we are doing optimizations
  std::shared_ptr<InstructionSequence> cur_hl_iseq(hl_iseq);

  if (m_optimize) {
    // High-level optimizations

    // Create a control-flow graph representation of the high-level code
    HighLevelControlFlowGraphBuilder hl_cfg_builder(cur_hl_iseq);
    std::shared_ptr<ControlFlowGraph> cfg = hl_cfg_builder.build();

    // Do local optimizations
    LocalOptimizationHighLevel hl_opts(cfg);
    cfg = hl_opts.transform_cfg();

    // Convert the transformed high-level CFG back to an InstructionSequence
    cur_hl_iseq = cfg->create_instruction_sequence();

    // The function definition AST might have information needed for
    // low-level code generation
    cur_hl_iseq->set_funcdef_ast(funcdef_ast);
  }

  // Translate (possibly transformed) high-level code into low-level code
  std::shared_ptr<InstructionSequence> ll_iseq = translate_hl_to_ll(cur_hl_iseq);

  if (m_optimize) {
    // ...could do transformations on the low-level code, possible peephole
    //    optimizations...
  }

  return ll_iseq;
}
```

The `LocalOptimizationHighLevel` class mentioned above would be a subclass
of `ControlFlowGraphTransform`, which would implement local optimizations
on each basic block in the high-level code. (See the
[Framework for optimizations](#framework-for-optimizations) section.)

### Ideas for optmizations to implement

Some ideas for code optimizations:

* Use machine registers for (at least some) local variables, especially loop variables
* Allocate machine registers for virtual registers
* Elimination of redundancies and inefficiencies in the high-level code,
  possibly using local value numbering
* Elimination of high-level instructions which assign a value to a virtual
  register whose value is not used subsequently (dead store elimination)
* Peephole optimization of the generated x86-64 assembly code to eliminate redundancies

The recommended way to approach the implementation of optimizations is
to look at the high-level and low-level code generated by your compiler,
and look for specific inefficiencies that you can eliminate.

We *highly* recommend that you keep notes about your work, so that you
have a written record you can refer to in preparing your report.

### Intermediate representations

You can implement your optimization passes as transformations of
`InstructionSequence` (linear IR) or `ControlFlowGraph` (control
flow graph IR).  We recommend that you implement transformations
constructively: for example, if transforming a `ControlFlowGraph`, the
result of the transformation should be a different `ControlFlowGraph`,
rather than an in-place modification of the original `ControlFlowGraph`.

If you do transformations of an `InstructionSequence`, you will need to
take control flow into account.  Any instruction that either

* is a branch, or
* has an immediate successor that is a labeled control-flow target

should be considered the end of a basic block. However, it's probably better
to create a `ControlFlowGraph`, since it makes basic blocks and control flow
explicit, and your analysis can then focus on doing local optimizations within
each basic block.

If you decide to use the `ControlFlowGraph` intermediate representation,
you can create it from an `InstructionSequence` as follows.
For high-level code:

```cpp
std::shared_ptr<InstructionSequence> iseq =
  /* InstructionSequence containing high-level code... */
HighLevelControlFlowGraphBuilder cfg_builder(iseq);
std::shared_ptr<ControlFlowGraph> cfg = cfg_builder.build();    
```

To convert a high-level `ControlFlowGraph` back to an `InstructionSequence`:

```cpp
std::shared_ptr<ControlFlowGraph> cfg = /* a ControlFlowGraph */
std::shared_ptr<InstructionSequence> result_iseq = cfg->create_instruction_sequence();
```

Note that the `create_instruction_sequence()` method is not guaranteed
to work if the structure of the `ControlFlowGraph` was modified.  I.e.,
we do not recommend implementing optimizations which change control flow.
Local optimizations (within single basic blocks) are recommended.

### Framework for optimizations

The [cfg\_transform.h](assign05/cfg_transform.h) and
[cfg\_transform.cpp](assign05/cfg_transform.cpp) source files demonstrate
how to transform a `ControlFlowGraph` by transforming each basic block.
The idea is to override the `transform_basic_block` member function.
In general, this class should be useful for implementing any local
(basic block level) optimization.

### Live variables analysis

The [live\_vregs.h](assign05/live_vregs.h) header file implements
[global live variables analysis](../lectures/Global_Optimization_Live_Analysis.pdf)
for virtual registers.  You may find this useful for determining which instructions
are safe to eliminate after local optimizations are applied.

You can see the results of live variables analysis by running the compiler program
using the `-L` command line option, e.g.

```
$ASSIGN05_DIR/nearly_cc -L input/example02.c
```

The command above should result in a printout of the basic blocks of the
high-level code for each function, with each instruction (and the beginning
and end of each basic block) being annotated with a set of virtual registers
which are alive.

To use `LiveVregs` as part of an optimization would look something like this:

```c++
// in the .h file
class MyOptimization : public ControlFlowGraphTransform {
private:
  LiveVregs m_live_vregs;

public:
  MyOptimization(const std::shared_ptr<ControlFlowGraph> &cfg);
  ~MyOptimization();

  virtual std::shared_ptr<InstructionSequence>
    transform_basic_block(const InstructionSequence *orig_bb);
};

// in the .cpp file
MyOptimization::MyOptimization(const std::shared_ptr<ControlFlowGraph> &cfg)
  : ControlFlowGraphTransform(cfg)
  , m_live_vregs(cfg) {
  m_live_vregs.execute();
}

MyOptimization::~MyOptimization() {
}

std::shared_ptr<InstructionSequence>
  MyOptimization::transform_basic_block(const InstructionSequence *orig_bb) {
  // ...
}
```

For example, let's say you are implementing dead store elimination as a subclass
of `ControlFlowGraphTransform`. In the `transform_basic_block` function
you might have code that looks something like this:

```c++
// LiveVregs needs a pointer to a BasicBlock object to get a dataflow
// fact for that basic block
const BasicBlock *orig_bb_as_basic_block =
  static_cast<const BasicBlock *>(orig_bb);

std::shared_ptr<InstructionSequence> result_iseq(new InstructionSequence());

for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
  Instruction *orig_ins = *i;
  bool preserve_instruction = true;

  if (HighLevel::is_def(orig_ins)) {
    Operand dest = orig_ins->get_operand(0);

    LiveVregs::FactType live_after =
      m_live_vregs.get_fact_after_instruction(orig_bb_as_basic_block, orig_ins);

    if (!live_after.test(dest.get_base_reg()))
      // destination register is dead immediately after this instruction,
      // so it can be eliminated
      preserve_instruction = false;
  }

  if (preserve_instruction)
    result_iseq->append(orig_ins->duplicate());
}

return result_iseq;
```

The code above would eliminate any instructions which assign to a virtual
register which is immediately dead.

### Generated code examples

The public test repository contains examples of unoptimized and optimized
code generated for the test programs.  Here are a few selections (see the
test repository for the full set):

Test program | HL unopt | HL opt | LL unopt | LL opt | LL peep
------------ | -------- | ------ | -------- | ------ | -------
[example02.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example02.c) | [example02.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code/example02.out) | [example02.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code_opt/example02.out) | [example02.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code/example02.S) | [example02.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_opt/example02.S) | [example02.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_peep/example02.S)
[example09.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example09.c) | [example09.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code/example09.out) | [example09.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code_opt/example09.out) | [example09.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code/example09.S) | [example09.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_opt/example09.S) | [example09.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_peep/example09.S)
[example28.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example28.c) | [example28.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code/example28.out) | [example28.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code_opt/example28.out) | [example28.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code/example28.S) | [example28.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_opt/example28.S) | [example28.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_peep/example28.S)
[example29.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example29.c) | [example29.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code/example29.out) | [example29.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code_opt/example29.out) | [example29.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code/example29.S) | [example29.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_opt/example29.S) | [example29.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_peep/example29.S)
[example30.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example30.c) | [example30.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code/example30.out) | [example30.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code_opt/example30.out) | [example30.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code/example30.S) | [example30.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_opt/example30.S) | [example30.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_peep/example30.S)
[example31.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example31.c) | [example31.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code/example31.out) | [example31.out](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_highlevel_code_opt/example31.out) | [example31.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code/example31.S) | [example31.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_opt/example31.S) | [example31.S](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/example_lowlevel_code_peep/example31.S)

The optimizations implemented are:

* Global assignment of callee-saved registers to local variables
  (with ranking placing higher priority on variables used in loops)
* Two iterations of
  * Constant propagation
  * Local value numbering and replacement of redundant computations with
    copies of previously computed values
  * Copy propagation
  * Removal of instructions in which the destination operand is immediately dead
* Local register allocation (using all argument registers that aren't needed
  for function calls)

In addition, the "LL peep" column shows the generated code with
low-level peephole optimizations enabled. These optimizations replace
less-efficient idioms in the generated low-level code with better
idioms, including

* use of index/scaled addressing mode for array element references
* loop conditions emitted using `cmp` followed immediately by a conditional jump
* elimination of unnecessary intermediate registers in arithmetic

See the [Peephole optimization](#peephole-optimization) section for some
additional information about peephole optimization.

As a measure of the improvement in runtime performance, these are the
running times (Linux on a Core i7-4790K CPU, averaged over 3 runs,
output redirected to `/dev/null`) of the
[Example 29](https://github.com/jhucompilers/fall2022-tests/blob/main/assign05/input/example29.c)
test program, which multiplies two 500x500 matrices of `long` values:

Version | Average running time
------- | --------------------
Unoptimized | 1.57 s
Optimized | 0.46 s
Optimized (peephole) | 0.24 s
gcc (-O2 optimization) | 0.12 s

The optimized code without peephole optimization is clearly much more efficient,
although given the very poor quality of the unoptimized code, this isn't too
surprising.

The optimized code with low-level peephole optimizations used to improve
instruction selection does signficantly better, although the same code
compiled using `gcc` with the `-O2` optimization level is still about
twice as fast.

### Peephole optimization

**Note**: This section is here just in case anyone is interested in trying
to improve their compiler's instruction selection using low-level peephole
optimization.  You are **not** expected to implement this, and the
code linked here should be considered to be experimental and very possibly
incorrect. You are absolutely free to ignore this section.

*Peephole optimization* is a fairly simple idea. A peephole optimizer
considers sequences of generated instructions, matching them against
patterns. When a pattern is matched, it is used to generate a more
efficient sequence of instructions.  The pattern can match opcodes
and/or operands in the original instruction sequence, in order
to preserve the semantics of the original sequence.

Here is an example of a peephole optimization implemented in the
reference solution:

```c++
  // Simplify 32 to 64 bit signed conversions
  pm(
    // match instructions
    {
      matcher( m_opcode(MINS_MOVL),   { m_mreg(A), m_mreg(B) } ),
      matcher( m_opcode(MINS_MOVSLQ), { m_mreg(B), m_mreg(C) } ),
      matcher( m_opcode(MINS_MOVQ),   { m_mreg(C), m_mreg(D) } ),
    },

    // rewrite
    {
      gen( g_opcode(MINS_MOVSLQ), { g_prev(A), g_prev(D) } ),
    },

    // Make sure B and C are dead afterwards
    "BC"
  ),
```

The idea in this code is that the `pm` function creates a peephole
sequence matcher. The first sequences of `matcher` objects matches
a specific sequence of instructions which implement a signed
conversion of a 32 bit integer to a 64 bit integer. When this
sequence is matched, it can be replaced by a single `movslq`
instruction.  However, because the original instruction sequence
defined values stored in the machine registers referred to by
the names `B` and `C` in the pattern, it is not correct to perform
the transformation unless the peephole optimizer knows that the
values in the machine registers matched by `B` and `C` are not
alive after the original sequence executes.

The original starter code includes an implementation of liveness
analysis for virtual registers in the high-level code.
If you are interested in using liveness information for machine
registers in the low-level code, which is helpful for determining
when low-level peephole optimizations are safe to apply, you can
try out the following code:

* [assign05-peep.zip](assign05-peep.zip)

As mentioned above, this code should be considered to be experimental.
You really should only be thinking about using it if you already
have a good set of optimizations implemented, and you should
**definitely** not predicate any critical functionality in your compiler
on the assumption that this code will work correctly. You have
been warned! Having said that, it has been tested to some extent,
and seems to work to a reasonable degree.

The `live_mregs.h`, `lowlevel_defuse.h`, and `lowlevel_defuse.cpp`
implement the liveness analysis for machine registers in the low-level
code. Also, the updated versions of `instruction.h` and
`instruction.cpp` contain new member functions which are needed.
The other files are optional, but contain some useful
improvements including adding support for the indexed/scaled
addressing mode, which is useful for array element access.

Note that the low-level def/use code assumes that `MINS_call`
instructions have a pointer to the `Symbol` object representing
the function being called. This is necessary in order to determine
which argument registers are used. You will need to modify the
`Instruction` class to support this, or implement some other
mechanism to allow the analysis to know which machine registers
are used by `call` instructions.

The "interesting" aspect of implementing peephole optimization is
implementing the pattern matching engine. The code example shown above
shows that peephole transformations can be specified declaratively,
which makes it relatively easy to add new patterns. However, it is
very possible to implement peephole sequence matching using ad-hoc
code.

## Submitting

To submit, create a zipfile containing

* all of the files needed to compile your program, and
* a PDF of your report in a file called `report.pdf`

Suggested commands:

```bash
make clean
zip -9r solution.zip Makefile *.h *.y *.l *.cpp *.rb report.pdf
```

The suggested command would create a zipfile called `solution.zip`.

Upload your submission to [Gradescope](https://www.gradescope.com) as **Assignment 5**.
