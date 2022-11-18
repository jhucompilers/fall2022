---
layout: default
title: "Assignment 5"
---

*This assignment description is somewhat preliminary, but has enough
information for you to get started. It will be updated.*

**Due date**: Friday, Dec 9th by 11pm

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

## Analysis and experiments

As you work on your optimizations, do experiments to see how effective
they are at improving the efficiency of the generated code.

You can use any test programs as a basis for these experiments.
One approach is to start with some very simple test programs, and then
work your way up to the benchmark programs (which are relatively complex.)

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
std::shared_ptr<InstructionSequence> result_iseq(new InstructionSequence());

for (auto i = orig_bb->cbegin(); i != orig_bb->cend(); ++i) {
  Instruction *orig_ins = *i;
  bool preserve_instruction = true;

  if (HighLevel::is_def(orig_ins)) {
    Operand dest = orig_ins->get_operand(0);

    LiveVregs::FactType live_after =
      m_live_vregs.get_fact_after_instruction(orig_bb, orig_ins);

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

Coming soon!

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
