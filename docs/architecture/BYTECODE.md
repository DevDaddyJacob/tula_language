# Tula Bytecode — the `.tbin` Format

**Bytecode** is Tula's compiled form of a program: a compact, linear list of simple
instructions for the virtual machine, produced by the code generator and consumed
by the VM. When written to disk it is a **`.tbin`** file.

```
 code generator  ──►  bytecode  ──►  virtual machine
                          │
                          └──►  .tbin file  ──►  (later) virtual machine
```

Bytecode is not a stage — it is the **interface** between the two halves of the
backend, and the engine's one durable artifact. It earns its own description
because so much of Tula's design rests on it being a real, stable format rather
than a private in-memory handoff.

## What it is, conceptually

Where source is text and the AST is a tree, bytecode is a **flat instruction
sequence** — much closer to what a machine executes than to what a person writes.
A bytecode program is essentially two things:

- **A stream of instructions.** Each instruction is a single small operation: push
  a constant, load or store a variable, perform an arithmetic or comparison
  operation, jump to another position, call a function, return, and so on. Some
  instructions carry a small operand — which constant, which variable, which jump
  target.
- **A pool of constants and metadata.** The literal values a program uses, the
  information the VM needs to run it, and the source positions needed for
  diagnostics all travel alongside the instructions.

Executing a program then becomes a straightforward walk through the instruction
stream, which is why bytecode is fast and simple to run compared with
re-interpreting a tree.

## Why it is a *file* format, not just a data structure

Bytecode exists in memory during a direct run, but it can also be **serialized to a
`.tbin` file**. Making it a real, persistable format is the single decision that
gives Tula its overall shape:

- It is the **seam** that lets compilation and execution be separated. The compiler
  can produce a `.tbin` now, and a runner can execute it later — on another
  machine, with no source and no compiler present.
- It lets the toolchain be **split into different executables** — one that only
  compiles (writes `.tbin`), one that only runs (reads `.tbin`), and one that does
  both — all agreeing on this one format.
- It is a **stable contract**. The code generator's whole job is to *produce* it;
  the virtual machine's whole job is to *consume* it. Neither needs to know
  anything about the other beyond this format.

## A self-contained program

A `.tbin` is meant to be **portable and self-sufficient**: everything needed to run
the program lives inside it — the instructions, the constants, and the metadata —
so running it never requires re-reading or re-parsing the original source. Because
it is a durable format that can outlive the exact toolchain that produced it, a
`.tbin` needs to be **identifiable and versioned**, so a runner can recognize a
file it understands and refuse one it does not. Treating the format as a long-lived
contract — not an implementation detail of the moment — is part of what makes
precompiled programs dependable.

## Where it sits

Bytecode is the **still point at the center of the backend**. Upstream, the code
generator lowers the AST into it; downstream, the virtual machine executes it; on
disk, it is a `.tbin` file that can bridge the two across time and space. Every one
of Tula's execution paths — compile-then-run, run-precompiled, run-directly —
converges on this single representation, which is precisely why the language
behaves identically no matter which path is taken.
