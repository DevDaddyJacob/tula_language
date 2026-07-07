# Tula Parser — Structure & the AST

The **parser** is the second stage of the engine. Where the scanner turns
characters into a flat list of tokens, the parser turns that flat list into a
**tree** — the shape that actually reflects how the program is organized.

```
 tokens (flat)  ──►  parser  ──►  AST (tree)  ──►  backend
```

Its one job: decide whether the tokens form a valid program and, if so, capture
that program's **structure** as an Abstract Syntax Tree (AST).

## From a line of tokens to a tree

A token stream is flat: `set`, `x`, `=`, `2`, `+`, `3`. That sequence says nothing
about grouping — it doesn't record that `2 + 3` is one expression whose value is
assigned to `x`, or that the whole thing is a single "set" statement. The parser
recovers that structure, producing something shaped like:

```
        set-statement
        ├── target: x
        └── value: ( + )
                   ├── 2
                   └── 3
```

Where the scanner answers *"what are the words?"*, the parser answers *"do these
words form valid sentences, and what is their structure?"*

## What the AST is

The **AST** is a tree of **nodes**, each node representing one construct in the
program:

- **Expressions** — things that produce a value: literals, a variable reference, an
  arithmetic or comparison operation combining sub-expressions, a function call, an
  increment or decrement, and so on.
- **Statements** — things that *do* something: defining or assigning a variable or
  constant, defining a function, control flow (`if`/`else`, `while`, `do`, `for`),
  loop control (`break`, `continue`), and `return`.
- **Blocks** — ordered groups of statements. A block is how the body of a function,
  loop, or conditional is represented — and how nesting is captured.

Nesting in the source becomes nesting in the tree: a `while` loop node contains a
block node, which contains the statement nodes of its body, some of which may
contain further blocks. The tree is "abstract" because it keeps what *matters* —
the structure and the meaningful pieces — and discards incidentals like
whitespace, comments, and the exact punctuation that merely guided parsing.

The grammar of the language defines which arrangements of tokens are valid and how
they nest; the parser is, in effect, that grammar made active. Each AST node also
carries the **source position** of the construct it represents, inherited from the
tokens it was built from, so later stages can still point back to the source.

## Grammar lives here — and so do syntax errors

The parser is the stage that knows the language's grammar, and therefore the stage
that detects **syntax errors** — a missing closing parenthesis, a statement that
can't legally begin with the token it starts with, an operator with nothing on its
right-hand side. In keeping with the engine-wide approach, these are **not**
crashes: a malformed program is expected input, and the parser reports the problem
as data (a syntax error tied to a position) rather than by aborting. A failure of
the engine itself remains a separate matter.

## The central interface

The AST is the **most important boundary in the whole engine** — the seam between
the *frontend* (reading and understanding source) and the *backend* (turning that
understanding into execution). It is the last representation that still mirrors the
source's structure; everything after it deals in progressively more
machine-oriented forms. Because the entire backend is written against the AST and
nothing earlier, the frontend can change *how* it produces the tree and the
backend can change *what* it does with the tree, as long as the tree between them
stays stable.

## Where it sits

Upstream, the parser depends only on the token stream — it never re-examines raw
characters. Downstream, it hands a complete AST to the code generator. That clean
separation — characters, then tokens, then a tree — is what lets each stage reason
about the program at exactly one level of abstraction, no higher and no lower.
