# Tula Virtual Machine — Executing Bytecode

The **virtual machine (VM)** is the second half of the backend and the end of the
pipeline. It takes bytecode and **runs it**, producing the program's actual
behavior — its output and its effects. If the rest of the engine is about
*understanding* a program, the VM is where the program finally *happens*.

```
 bytecode (or .tbin)  ──►  virtual machine  ──►  output / effects
```

## A machine defined in software

A physical CPU executes instructions against registers and memory. The VM is the
same idea implemented in software: it executes Tula's bytecode instructions against
**runtime state** that it manages itself. It is "virtual" because it is not tied to
any real processor — the same bytecode runs identically wherever the VM does.

At its core the VM is a simple loop: **fetch** the next instruction, **decode**
what it is, **execute** its effect on the runtime state, and advance — repeating
until the program ends. Every instruction the code generator can emit has a
corresponding, well-defined action here.

## What the VM keeps track of

To run a program, the VM maintains the runtime state that the flat instruction
stream operates on, including:

- **A working area for values.** Intermediate results — the `2` and `3` on their
  way to becoming `5` — live in a scratch area that instructions push onto and pull
  from as they compute.
- **Variable storage.** The current values of variables and constants, organized by
  scope, so the stores and loads emitted by the code generator have somewhere to
  act.
- **Control-flow position.** Which instruction comes next. Jumps simply change this
  position, which is how the flattened `if` / `while` / `for` structures actually
  branch and loop at run time.
- **The call stack.** When a function is called, the VM records where to return to
  and keeps each call's local state separate, so functions can call other functions
  (and themselves) without interfering.

## The one place meaning lives

The VM is where the language's **runtime semantics** are defined — what it *means*
to add two values, to compare them, to call a function, to loop. This is the single
most important consequence of the backend's design: because **every** execution
path funnels through the same bytecode and the same VM, those semantics exist in
exactly one place. A program compiled ahead of time to a `.tbin` and a program run
straight from source are, by the time they execute, doing the very same thing in
the very same machine. They cannot drift apart, because there is only one
definition of "run."

## Runtime errors

Some problems can only surface while a program runs — not everything can be caught
by scanning or parsing. These **runtime errors** are distinct from the syntax
errors the frontend reports. Thanks to the source positions carried all the way
from the reader, through the tree, and into the bytecode, the VM can still tie a
runtime problem back to the place in the original source that caused it — even
though it is working purely with instructions far removed from the text.

## Where it sits

The VM sits at the very end of the pipeline, the mirror image of the reader at the
start: the reader turns a file into a stream the engine can understand, and the VM
turns the engine's understanding back into things that actually happen. Its input
is bytecode — whether freshly generated in memory or loaded from a `.tbin` — and
its output is the running program itself. Everything upstream exists to hand this
stage a correct, compact description of what to do; the VM is what does it.
