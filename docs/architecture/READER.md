# Tula Reader — the Character Source

Every stage of the Tula engine ultimately works from source text, but only one
part actually touches a source file: the **reader**. Its job is to turn a source
file into a **stream of characters**, handed out one at a time, on demand — while
keeping track of *where* in the source each character came from.

```
 source file  ──►  reader  ──►  characters (+ line/column)  ──►  scanner
```

The reader is the floor the whole engine stands on. Everything above it — the
scanner, parser, and beyond — is written against a simple character stream, not
against files, encodings, or disk. That single boundary is what keeps the rest of
the engine free of input concerns.

## The contract

The reader exposes the source as a **cursor** over characters, with two essential
operations:

- **Peek** — look at an upcoming character *without* moving the cursor. The reader
  allows looking a short distance ahead, not only at the very next character.
- **Consume** — take the next character and advance the cursor.

That is the entire vocabulary a consumer needs. It never asks "how big is the
file?" or "jump to position N"; it only ever asks "what's next?" and "give me the
next one." This keeps consumers simple and makes the source feel like an endless
supply of characters that eventually runs out.

## Two ideas that shape everything downstream

**1. Streaming, pull-based input.**
The reader does not load the whole file into memory. It pulls the source through
in fixed-size chunks and refills as the cursor advances. Because input is pulled
rather than pushed, each stage only asks for the next character when it is actually
ready for it. The practical consequence is that the engine never assumes a program
is small enough to fit in memory — a very large source is processed the same way as
a tiny one. This is the origin of the engine-wide "streaming from the bottom up"
principle.

**2. Position tracking.**
As characters flow past, the reader maintains the current **line and column**.
This is the single point where source positions enter the system. When the scanner
forms a token it stamps it with the reader's current position; the parser carries
those positions into the tree; the code generator carries them into the bytecode.
Every precise "error at line X, column Y" message anywhere in the engine traces
back to the bookkeeping the reader does here. Attaching position at the very
bottom — rather than trying to reconstruct it later — is what makes accurate
diagnostics possible end to end.

## Lookahead, and why it matters

The reader's ability to peek *ahead* (not just at the immediate next character) is
what lets the scanner make decisions with a tiny, fixed amount of context — for
example, telling a one-character operator from its two-character cousin, or a
minus sign from the start of a negative number, by glancing at the following
character before committing. Providing bounded lookahead at the character level
keeps the higher stages from having to buffer the whole input or backtrack.

## Where it sits

The reader is the boundary between **"a file"** and **"a stream of characters."**
Upstream is bytes, encodings, and input; downstream is a clean, positioned
character stream that every later stage can rely on without knowing where it came
from. Getting this boundary right — uniform, positioned, streamed — is what lets
the rest of the engine ignore the existence of files entirely.
