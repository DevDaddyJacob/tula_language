# Tula Engine — Compilation & Execution Pipeline

This document describes the architecture of the **Tula engine**: the staged
pipeline that turns Tula source code into a running program. It is a high-level
design document — it explains the stages, the boundaries between them, and how
the pieces fit together. For the internals of any one stage, see that stage's own
design document.

---

## The big picture

The engine is a **pipeline**. Source text enters at one end, and each stage
transforms it into a progressively more structured representation until it can be
executed:

```
   source         characters        tokens             AST            bytecode           result
  ┌────────┐ read ┌────────┐ scan ┌─────────┐ parse ┌────────┐ emit ┌──────────┐ exec ┌────────┐
  │ .tula  │ ───► │ reader │ ───► │ scanner │ ────► │ parser │ ───► │ codegen  │ ───► │   VM   │ ──► output
  └────────┘      └────────┘      └─────────┘       └────────┘      └──────────┘      └────────┘
  └──────────────────────── frontend ──────────────────────┘       └───────────── backend ──────────┘
```

The guiding principle is **staged separation of concerns**: each stage knows only
how to consume the output of the stage before it and produce input for the stage
after it. The scanner deals in characters and knows nothing of grammar; the
parser deals in tokens and knows nothing of how a file is read; the VM deals in
bytecode and knows nothing of source text. The *interfaces between stages* — the
token stream, the AST, the bytecode — are the real architecture. Get those clean
and each stage can be built, tested, and replaced independently.

### Three execution methods

That single pipeline is driven in three different ways. Each is a different path
through it (or into it), and together they are why Tula is at once a compiler and
an interpreter:

```
  1. Compile        .tula ─► frontend ─► code gen ─► .tbin            (tulac, tula)
                    bytecode is written to disk, then the run stops

  2. Run compiled   .tbin ─► VM ─► output                            (tular, tula)
                    the frontend is skipped entirely

  3. Run directly   .tula ─► frontend ─► code gen ─► VM ─► output     (tula)
                    bytecode stays in memory; no .tbin is written
```

Here *frontend* is the scan → parse work that yields an AST, and *code gen* is the
code generator that lowers that AST to bytecode. The `.tbin` file is the artifact
that joins method 1 to method 2: `tulac` produces it, `tular` consumes it. Method
3 fuses both halves in memory and never touches disk. All three funnel through the
**same** code generator and the **same** VM, so the language behaves identically
no matter which one is used.

### The destination: one frontend, a forking backend

The three execution methods above are packaged into three executables, each
carrying only the slice of the pipeline it needs — plus a debug build used during
development:

| Executable | Role |
|------------|------|
| **`tula`** | The full toolchain. Runs any of the three methods: compile `.tula` to `.tbin`, run a `.tbin`, or run a `.tula` source directly in memory. |
| **`tulac`** | The **compiler**. Runs the frontend and code generator, then stops — reads `.tula`, writes `.tbin`, never executes. |
| **`tular`** | The lean **runner**. The tail of the pipeline only — loads a `.tbin` and runs the VM. It carries none of the frontend, which is exactly why it can be small. |
| **`tulad`** | A **debug build of `tula`** — the same full toolchain compiled in debug mode with extra checks and instrumentation. It backs the automated testing framework (where stages can be exercised in isolation) and is the build reached for during development and troubleshooting. |

The architectural consequence is that **the frontend is shared and the backend is
a fork**. Scanning and parsing are identical no matter which method runs — they
always produce an AST. What differs is only what consumes it, so each executable
is a different subset of one pipeline rather than a program of its own.

This works because the pipeline has two clean seams: one **after the AST**
(frontend vs. backend) and one **at the `.tbin` bytecode** (code generator vs. VM).
The `.tbin` format is a durable, serializable boundary — not just an in-memory
handoff — which is what lets `tular` take only the VM side of it, `tulac`
everything up to it, and `tula` the whole thing.

---

## The stages

### 0. Input — the character source

Before any language processing happens, source has to be read. The engine never
manipulates files directly; it reads through a **buffered reader** that streams a
file in fixed-size chunks and exposes a small character cursor: *peek* (look ahead
without consuming), *read* (consume), and *consume*. Crucially, the reader also
tracks the current **line and column** as characters flow past, so every later
stage can report *where* in the source something happened.

This is a deliberate design choice: the engine is **streaming and pull-based**
from the very bottom. Nothing loads the whole source into memory; each stage asks
for the next unit only when it needs it.

### 1. Scanning (lexing) — characters ➜ tokens

The **scanner** groups characters into **tokens** — the atomic lexical units of
the language (identifiers, keywords, typed literals, operators, and meta markers
like end-of-stream and error). It strips whitespace and comments, recognizes the
*shapes* of lexemes, and stamps each token with its source position.

The scanner has no notion of grammar — it does not know whether a token sequence
is valid, only what each token *is*. Its output is a flat, position-tagged token
stream that is far simpler to work with than raw text.

### 2. Parsing — tokens ➜ AST

The **parser** is the stage that imposes *structure*. It consumes the token stream
and, guided by the language's grammar, assembles tokens into an **Abstract Syntax
Tree (AST)** — a tree of nested statements and expressions that captures what the
program *means* structurally: this is a function definition, that is a
`while` loop whose body contains these statements, this expression is a
comparison of those two operands.

Where the scanner answers "what are the words?", the parser answers "do these
words form valid sentences, and what is their structure?" It is the stage that
detects *syntax* errors (a missing `)`, a statement that can't start with this
token).

The AST is the pipeline's **central interface** — the boundary between frontend
and backend. It is the last representation that still mirrors the source's
structure, and the input to everything the backend does.

### 3. Backend — AST ➜ bytecode ➜ execution

Everything past the AST is the **backend**, and this is where Tula's dual nature
lives. It has two halves, separated by the `.tbin` bytecode format:

- **The code generator (AST ➜ bytecode).** The code generator walks the AST and
  lowers it into a compact, linear **bytecode** — a sequence of simple
  instructions for a virtual machine. This is the "compile" half: the expensive
  scan → parse → generate work is done once and can be serialized to a `.tbin`
  file for reuse.

- **The virtual machine (bytecode ➜ execution).** The VM executes bytecode to
  produce the program's actual behavior. It obtains that bytecode either by
  loading a `.tbin` file or by taking freshly-generated bytecode straight from the
  code generator in memory.

`.tbin` is the pivot between the two halves. Because it is a real, serializable
format — not just an in-memory handoff — the two halves can be split across
executables (`tulac` generates, `tular` runs) or fused in one (`tula`). And
because every execution path funnels through the *same* bytecode and the *same*
VM, the language's runtime semantics are defined in exactly one place, whether the
program was pre-compiled with `tulac` or run straight from source with `tula`.

---

## Recurring design themes

A few principles run through every stage and hold the pipeline together at a
conceptual level:

- **Clean interfaces over clever stages.** The value is in the boundaries — the
  token stream, the AST, the bytecode. Each is a simple, well-defined handoff, and
  that is what lets stages be developed, tested, and replaced on their own.

- **Streaming from the bottom up.** Source is pulled through in chunks and each
  stage asks for the next unit only when it needs it, so the engine never assumes
  a whole program fits in memory at once.

- **Positions travel with the data.** A source location is attached at the reader
  and carried by every representation after it — tokens, AST, and bytecode — so an
  error can always point back to the exact spot in the original source, even after
  compilation to `.tbin`.

- **One place for meaning.** Because every path converges on the same bytecode and
  the same VM, the language's runtime behavior is defined once. Compiling ahead of
  time and running directly can never diverge.

The engineering conventions that realize these themes — memory ownership, error
handling, and how the executables are built — are developer concerns rather than
architecture, and are covered separately in the project's guidelines.
