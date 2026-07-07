# Tula Scanner — Design & Architecture

The **scanner** (also called a lexer or tokenizer) is the first stage of the Tula
engine. Its job is narrow and well-defined: turn a flat stream of *characters*
into a flat stream of *tokens*.

```
"set x = 42"  ──►  [ set ] [ x ] [ = ] [ 42 ] [ <eos> ]
  characters          tokens
```

That is the whole contract. The scanner has **no understanding of grammar** — it
doesn't know that `set` expects a name, or that `=` needs something on both
sides. It only recognizes the *shapes* of individual lexical units and labels
them. Deciding whether a sequence of tokens is a valid program is the parser's
job, a later stage.

This separation is the central design idea: by collapsing raw text (whitespace,
comments, multi-character spellings, escape sequences, numeric suffixes) into a
clean sequence of uniform, position-tagged tokens, every downstream stage gets to
work with something far simpler than characters.

The scanner has two concerns, and this document follows them in turn:

- **What a token is** — the vocabulary and the shape of a token.
- **How tokens are produced** — the recognition process.

---

## Part 1 — What a token is

### The concept

A **token** is the smallest meaningful piece of source text — a "word" in the
language. Tula's tokens fall into a handful of conceptual families:

| Family | What it represents | Examples |
|--------|--------------------|----------|
| **Literals** | A concrete value written directly in the source | `42`, `3.14`, `"hello"`, `'a'`, `true` |
| **Identifiers** | A user-chosen name | `x`, `myFunction`, `counter` |
| **Keywords** | Names reserved by the language | `set`, `if`, `while`, `function`, `return` |
| **Operators** | Symbols that combine or punctuate | `+`, `==`, `(`, `}`, `,` |
| **Meta tokens** | Markers that aren't source text at all | end-of-stream, error |

Two families deserve special note because they shape the design:

- **Literals carry a type.** Tula distinguishes numeric widths and signedness at
  the *lexical* level: `42b` is an 8-bit int, `42s` a 16-bit int, `42` a 32-bit
  int, `42l` a 64-bit int, each with an unsigned variant, plus fractional `float`
  and `double` forms. So the scanner doesn't emit a generic "number" token — it
  emits a specifically-typed one: an 8-bit integer, a 32-bit unsigned integer, a
  double, and so on. The type is decided from the literal's shape and its suffix.

- **Meta tokens make the stream self-terminating and self-describing.** Rather
  than signalling "done" or "something went wrong" through out-of-band channels,
  the scanner emits an end-of-stream token at the end and an error token (carrying
  a human-readable message) when it hits something it can't lex. Consumers just
  keep reading tokens until they see one of these.

### The shape of a token

Every token, regardless of family, has the **same shape**. A token is not just a
label; it carries three things:

- **Its kind** — which entry in the token vocabulary this is (a plus sign, an
  identifier, an integer literal, and so on).
- **Its text** — the exact slice of source that produced it. For an identifier or
  literal this is the meaningful payload (`myVar`, `42`); for an error token it is
  a human-readable message.
- **Its position** — the line and column where the lexeme begins, captured so that
  later stages and error messages can point back into the source.

Keeping every token in one uniform shape is a deliberate simplification: every
consumer — the parser, the collection that stores tokens, any tool that prints
them — handles all tokens the same way, and only the *kind* varies.

### The token vocabulary: one source of truth

The full set of token kinds *is* the language's lexical vocabulary. Each kind ties
together a few things that must always agree: a canonical spelling (`+` for a plus
sign, `function` for that keyword), sometimes an alternate spelling, and a
classification (is this a literal? a keyword? an operator? a meta marker?).

The design keeps this vocabulary as a **single canonical list**, defined once, from
which every view of it is derived — the set of kinds, their spellings, and their
classifications all trace back to the same place. The payoff is twofold: the
vocabulary can never fall out of sync with itself, and adding or changing a token
is a change in exactly one place. The **alternate spelling** is how the language
offers long/short synonyms for the same keyword — `function`/`func`,
`variable`/`var`, `constant`/`const`, `define`/`def`.

### Collecting tokens

Because the number of tokens in a source file isn't known in advance, the scanner
accumulates them in an **ordered, growable collection** that expands as needed. As
the scanner runs, this collection becomes the complete lexical record of the
input, in source order — the flat token stream the parser will later consume.

---

## Part 2 — How the scanner works

### The mental model

At its core the scanner is a loop over a **character cursor**. It reads through the
reader, which supplies characters on demand and tracks the current line and
column. The scanner can *peek* ahead (look without consuming) and *consume*
(advance the cursor). That small cursor is the only way the scanner sees its
input.

The reader hands the scanner one thing for free: because position is tracked as
characters flow past, the scanner can stamp each token's origin simply by noting
the current position when a lexeme begins.

### One token at a time

The scanner is fundamentally *pull-based*: something asks for the next token, and
the scanner produces exactly one. Producing a single token follows the same shape
every time:

1. **Discard the noise.** Skip any leading whitespace *and comments*. Comments
   carry no meaning to later stages, so the scanner treats them as "honorary
   whitespace" and drops them entirely — they never become tokens.

2. **Check for the end.** If the input is exhausted, emit the end-of-stream token.

3. **Peek one character and decide what's coming.** A single character of
   lookahead is enough to know *which kind* of token starts here:
   - a letter or underscore → an **identifier or keyword**
   - a digit (or a minus sign immediately followed by a digit) → a **number**
   - a quote → a **string**, an apostrophe → a **character**
   - a symbol → an **operator**
   - anything else → an **error**

4. **Hand off to the matching recognizer**, which consumes exactly the characters
   belonging to that one token and builds it.

5. **Record and return.** The finished token is appended to the token collection
   and returned to the caller.

A convenience wrapper repeats this until it sees an end-of-stream or error token,
which is how the whole input gets lexed in one pass.

### How each kind of token is recognized

The interesting design decisions live in the individual recognizers:

- **Identifiers vs. keywords.** The scanner first reads a maximal run of identifier
  characters (a letter or underscore, followed by any mix of letters, digits, and
  underscores), then asks: *is this run exactly one of the reserved keywords?*
  Only an exact spelling match becomes a keyword token; everything else is a plain
  identifier. This "read the word, then classify it" approach means keywords
  aren't special-cased in the character loop — they're just identifiers that
  happen to match a reserved spelling.

- **Numbers are typed as they're read.** A numeric literal is examined in two
  conceptual passes: first the scanner *looks ahead* to work out the concrete
  type — is there a leading minus (signed/negative)? a decimal point
  (floating-point)? a width or sign suffix? — and rejects contradictory
  combinations (an unsigned marker on a fractional number, a negative unsigned) as
  errors. Only once the type is known does it read the digits into a space sized
  for that type, catching literals too long to fit. The result is a token that
  already knows it is, say, an unsigned 16-bit integer.

- **Operators and lookahead.** Most operators are a single character. The few that
  have a two-character form (`==`, `!=`, `>=`, `<=`, `++`, `--`) are resolved with
  **one more character of lookahead**: see `=`, glance at the next character, and
  choose `==` or `=` accordingly. The same one-character-lookahead trick resolves
  the genuinely ambiguous symbols — a minus is subtraction *unless* a digit
  follows (then it's the sign of a number), and a slash is division *unless* it
  opens a comment.

- **Strings and characters** are read between their delimiters, with string
  scanning understanding backslash escapes so that an escaped quote doesn't end
  the string prematurely.

### Errors are just tokens

The scanner distinguishes two very different failure modes, and handles them
differently by design:

- **Malformed source** (an unexpected character, a bad numeric suffix, an
  over-long identifier or string) is *expected* — source code has bugs. It doesn't
  stop the scanner or crash the program; it produces an error token carrying a
  readable message and a source position. A driver reading the stream stops at the
  first one, but the mechanism is ordinary token flow.

- **Internal failures** (running out of memory, an impossible internal state) are a
  different matter entirely: they are not the program's fault and are never
  represented as tokens. The engine stops cleanly rather than inventing a token
  for a fault that isn't in the source.

This split keeps the token stream honest: an error token always means "the source
was wrong here," never "the scanner itself broke."

---

## Where the scanner sits

```
characters  ──►  scanner  ──►  tokens  ──►  parser
```

The scanner is the boundary between **"text"** and **"structure."** Everything
upstream deals in characters; everything downstream deals in tokens. Getting that
boundary clean — uniform tokens, positions preserved, noise stripped, errors
in-band — is the entire point of the design.
