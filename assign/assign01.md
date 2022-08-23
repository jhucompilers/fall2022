---
layout: default
title: "Assignment 1"
---

# Interpreter part 1: expression evaluation

In this assignment you will implement an interpreter which serves
as a calculator, reading a sequence of statements containing expressions
to be evaluated, and then analyzing and evaluating them.

## Getting started

TODO

## Grammar changes

These are the grammar productions implemented in the starter code
(which is adapted from the [astdemo](https://github.com/daveho/astdemo)
example code):

<div class="highlighter-rouge"><pre>
Unit &rarr; Stmt
Unit &rarr; Stmt Unit
Stmt &rarr; E ;
E    &rarr; T E'
E'   &rarr; + T E'
E'   &rarr; - T E'
E'   &rarr; ε
T    &rarr; F T'
T'   &rarr; * F T'
T'   &rarr; / F T'
T'   &rarr; ε
F    &rarr; number
F    &rarr; ident
F    &rarr; <code>(</code> E <code>)</code>
</pre></div>

You will add the following productions to the grammar (implement these
in `Parser2`):

<div class="highlighter-rouge"><pre>
Stmt &rarr; <code>var</code> ident <code>;</code>
A    &rarr; ident <code>=</code> A
A    &rarr; L
L    &rarr; R <code>||</code> R
L    &rarr; R <code>&amp;&amp;</code> R
L    &rarr; R
R    &rarr; E <code>&lt;</code> E
R    &rarr; E <code>&lt;=</code> E
R    &rarr; E <code>&gt;</code> E
R    &rarr; E <code>&gt;=</code> E
R    &rarr; E <code>==</code> E
R    &rarr; E <code>!=</code> E
R    &rarr; E
</pre></div>

Note that the symbols beginning with an upper-case letter
(e.g., `Stmt`, `A`, `L`, `R`, `E`, `E'`) are nonterminals, and all other symbols
(e.g., `(`, `)`, `=`, `==`, `<`, etc.) are terminals.

You will also need to change the production

<div class="highlighter-rouge"><pre>
Stmt &rarr; E ;
</pre></div>

to

<div class="highlighter-rouge"><pre>
Stmt &rarr; A ;
</pre></div>

You will need to add new AST node types to `ast.h` and `ast.cpp`
to represent the new constructs added to the language.

## Scanner changes

You will need to implement changes to the lexical analyzer (in `lexer.cpp`,
as well as `token.h`) to add the following new tokens:

```
var
=
||
&&
<
<=
>
>=
==
!=
```

Note that the two-character tokens where the first character is also
a valid token — for example, `>=` — will require special handling.
You might want to add new helper functions to the lexer.
