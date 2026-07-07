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

The code lives in `src/engine/scanner/`, split into two concerns:

- **`token.*`** — *what a token is* (the vocabulary and its in-memory shape).
- **`scanner.*`** — *how tokens are produced* (the recognition process).

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
  int, `42l` a 64-bit int, each with an unsigned `u` variant, plus `float` and
  `double` for fractional numbers. So the scanner doesn't emit a generic
  "number" token — it emits a specifically-typed one (`<int8>`, `<uint32>`,
  `<double>`, …). The type is decided from the literal's shape and its suffix.

- **Meta tokens make the stream self-terminating and self-describing.** Rather
  than signalling "done" or "something went wrong" through out-of-band return
  codes, the scanner emits an `<eos>` (end-of-stream) token at the end and an
  `<error>` token (carrying a human-readable message) when it hits something it
  can't lex. Consumers just keep reading tokens until they see one of these.

### How a token is represented in code

Every token, regardless of family, is the **same struct**. A token is not just a
label — it also remembers *what* text produced it and *where* it came from:

```c
typedef struct tula_token {
    token_type_t type;          // which token this is (e.g. TOK_PLUS)
    uint32_t     line;          // source position: line ...
    uint32_t     column;        // ... and column of where it starts
    char*        content;       // the exact lexeme text ("42", "myVar", "+")
    size_t       contentLength;
} token_t;
```

- `type` places the token in the vocabulary.
- `content` preserves the original text. For an identifier or literal this is the
  meaningful payload (`"myVar"`, `"42"`); for an error token it's the diagnostic
  message.
- `line` / `column` are captured at the **start** of the lexeme so that later
  stages and error messages can point back into the source.

Keeping every token in one uniform shape means the token array, the parser, and
the debug printer all handle every token identically — the `type` field is the
only thing that varies.

### The token vocabulary: one source of truth

The full set of token types *is* the language's lexical vocabulary, so it must
stay internally consistent: the enum, the printable spelling of each token, and
its classification flags all have to agree. Rather than maintain those in
parallel by hand (and inevitably drift), the design declares each token **once**
and derives everything else from that declaration.

Every token is a single row in the `DEFINE_TOKENS` list, carrying its identifier,
its canonical spelling, an optional alternate spelling, and four classification
flags (`isMeta`, `isPrimitive`, `isKeyword`, `isOperator`):

```c
def(TOK_PLUS,     "+",        NULL,   false, false, false, true )
def(TOK_FUNC,     "function", "func", false, false, true,  false)
def(TOK_INT32,    "<int32>",  NULL,   false, true,  false, false)
```

From this one list, the compiler generates the `token_type_t` enum and a family
of parallel lookup arrays (spelling, alternate spelling, and one array per
classification flag), all indexed by token type. The *alternate spelling* column
is how the language offers long/short synonyms — `function`/`func`,
`variable`/`var`, `constant`/`const`, `define`/`def`.

The practical rule that falls out of this design: **to add or change a token, you
edit only the `DEFINE_TOKENS` list.** The enum and every lookup table regenerate
automatically. (This is the project-wide "X-macro" pattern; the same technique
drives the `tulad` test-mode table.)

### Collecting tokens: the token array

Because the number of tokens in a source file isn't known ahead of time, the
scanner accumulates them in a **growable array** (`arr_token_t`) — a simple
`count` / `capacity` / `values` vector that doubles its capacity on demand. The
scanner owns this array and appends every token it produces to it, so the array
becomes the full lexical record of the input, in order.

---

## Part 2 — How the scanner works

### The mental model

At its core the scanner is a loop over a **character cursor**. It doesn't load the
whole file into memory; it reads through a buffered reader that supplies
characters on demand and tracks the current line and column as it goes. The
scanner can *peek* ahead (look without consuming) and *consume* (advance the
cursor). That two-operation cursor is the only way the scanner sees the input.

The reader hands the scanner one more thing for free: because it tracks
line/column as characters flow past, the scanner can stamp each token's origin
just by reading the reader's current position when a lexeme begins.

### One token at a time

The scanner is fundamentally *pull-based*: something asks for the next token, and
the scanner produces exactly one. Producing a single token follows the same
shape every time:

1. **Discard the noise.** Skip any leading whitespace *and comments*. Comments
   (`// line` and `/* block */`) carry no meaning to later stages, so the scanner
   treats them as "honorary whitespace" and drops them entirely — they never
   become tokens.

2. **Check for the end.** If the input is exhausted, emit the `<eos>` meta token.

3. **Peek one character and decide what's coming.** A single character of
   lookahead is enough to know *which kind* of token starts here:
   - a letter or `_` → an **identifier or keyword**
   - a digit (or a `-` immediately followed by a digit) → a **number**
   - a `"` → a **string**, a `'` → a **character**
   - a symbol → an **operator**
   - anything else → an **error**

4. **Hand off to the matching recognizer**, which consumes exactly the characters
   belonging to that one token and builds it.

5. **Record and return.** The finished token is appended to the token array and
   returned to the caller.

A convenience wrapper repeats this until it sees an `<eos>` or `<error>` token,
which is how the whole input gets lexed in one call.

### How each kind of token is recognized

The interesting design decisions live in the individual recognizers:

- **Identifiers vs. keywords.** The scanner first reads a maximal run of
  identifier characters (`[A-Za-z_][A-Za-z0-9_]*`), then asks: *is this run
  exactly one of the reserved keywords?* Only an exact spelling match becomes a
  keyword token; everything else is a plain identifier. This "read the word, then
  classify it" approach means keywords aren't special-cased in the character
  loop — they're just identifiers that happen to match a reserved spelling.

- **Numbers are typed as they're read.** A numeric literal is scanned in two
  conceptual passes: first the scanner *looks ahead* to work out the concrete
  type — is there a leading `-` (signed/negative)? a `.` (floating-point)? a type
  suffix like `b`, `s`, `l`, `u`, or `d`? — and rejects contradictory
  combinations (a `u` on a float, a `d` on an integer, a negative unsigned) as
  errors. Only once the type is known does it copy the digits into a buffer sized
  for that type, catching literals too long to fit. The result is a token that
  already knows it's, say, a `<uint16>`.

- **Operators and lookahead.** Most operators are a single character. The few
  that have a two-character form (`==`, `!=`, `>=`, `<=`, `++`, `--`) are resolved
  with **one more character of lookahead**: see `=`, peek at the next character,
  and choose `==` or `=` accordingly. The same one-char-lookahead trick resolves
  the genuinely ambiguous symbols — `-` is a minus operator *unless* a digit
  follows (then it's the sign of a number), and `/` is division *unless* it opens
  a comment.

- **Strings and characters** are read between their delimiters, with string
  scanning understanding backslash escapes so that an escaped quote doesn't end
  the string prematurely.

### Errors are just tokens

The scanner distinguishes two very different failure modes, and handles them
differently by design:

- **Malformed source** (an unexpected character, a bad numeric suffix, an
  over-long identifier or string) is *expected* — source code has bugs. These
  don't stop the scanner or crash the program; they produce an `<error>` token
  carrying a readable message and a source position. The batch driver stops at
  the first one, but the mechanism is ordinary token flow.

- **Internal failures** (out of memory, an impossible internal state) are *not*
  recoverable and are never represented as tokens. They route through the
  project's central exit helpers so that cleanup and exit codes stay consistent,
  in keeping with the codebase rule of never calling `exit()` directly.

This split keeps the token stream honest: an `<error>` token always means "the
source was wrong here," never "the scanner itself broke."

### Ownership

The ownership chain is deliberately simple and top-down: the scanner owns the
reader (its input) and the token array (its output); the array owns the token
structs; each token owns its copied text. Tearing down the scanner frees the
entire graph in one call. Tokens handed back to callers are *borrowed* — they
stay valid as long as the scanner lives.

---

## Where the scanner sits

```
source file
     │
     ▼
buffered reader ──(characters)──► scanner ──(tokens)──► parser ──► AST ──► ...
                                     │
                                     └─ owns the growing token array
```

The scanner is the boundary between "text" and "structure." Everything upstream
deals in bytes; everything downstream deals in tokens. Getting that boundary
clean — uniform tokens, positions preserved, noise stripped, errors in-band — is
the entire point of the design.

### Exercising it

The `tulad` debugging build can run the scanner in isolation
(`tulad --mode scanner --input-file <path>`), printing the token stream it
produces. That same capability backs the golden-file integration tests under
`test/integration_tests/data/it_scanner/`, which lex fixture inputs and diff the
resulting token stream against expected output. See `docs/TESTING.md`.
