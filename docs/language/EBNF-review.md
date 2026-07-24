# EBNF Grammar Review

A fresh read of `docs/language/EBNF.md`. Findings are grouped by severity. Line
numbers refer to the current file.

## Summary

The grammar is well organized and mostly internally consistent: a clean
lexical layer, a proper precedence-tiered expression grammar, keyword-gated
declarations, and structural (rather than comment-only) enforcement of several
rules. The issues below are what I'd resolve before the grammar drives a parser
— the first two are outright blockers.

---

## Blocking issues

### B1. `if`/`else` is unreachable — `comparison_statement` is not a `statement`

`comparison_statement_single` and `comparison_statement` are defined (lines
222–225) but **neither appears in the `statement` union** (lines 252–268). As a
result, conditionals cannot occur inside a `block` — an `if` statement is
ungrammatical anywhere in the language. `while`, `do/while`, and `for` are all
wired into `statement`; `if` was simply left out.

**Recommendation:** add `| comparison_statement` to the `statement` union (the
`comparison_statement` form already subsumes the single/else-if/else cases, so
`comparison_statement_single` does not need its own entry).

### B2. No start symbol / top-level program rule

The outermost construct is `block` (line 274), which is wrapped in `{ }`. There
is no `program`/`module`/`compilation_unit` rule describing a whole source file
as a bare sequence of statements. As written, a valid Tula file would have to be
a single brace-wrapped block, which is almost certainly not the intent.

**Recommendation:** add an explicit start rule, e.g.
`program = {statement} [return_statement] ;` (mirroring `block` without the
braces), and note it as the grammar's entry point.

---

## Correctness / design gaps

### C1. String and char literals cannot contain spaces, and have no escapes

`char_all = ? all visible characters ?` (line 25). A space (and tab) is not a
*visible* character, so:

- `literal_string = '"' {char_all - '"'} '"'` (line 48) cannot match `"hello world"` — the space is excluded.
- `literal_char` (line 50) cannot match `' '`.

Separately, neither literal has an escape mechanism beyond `\'` in `literal_char`:
a string cannot contain an embedded `"` or newline, and there are no `\n`, `\t`,
`\\`, `\0` escapes.

**Recommendation:** widen the string/char character class to "all characters
except the closing delimiter and newline" (keep newline out if strings are
single-line), and define an escape sequence production shared by both literals
(at minimum `\"`, `\'`, `\\`, `\n`, `\t`).

### C2. No unary minus — negative numbers are inexpressible

`literal_integer`/`literal_decimal` are built from unsigned `digits`, and
`expression_unary` (line 152) offers only `not`, `++`, `--` — no arithmetic
negation. `-` exists solely as a binary operator (`expression_additive`). So
`-5` cannot be parsed; you must write `0 - 5`.

**Recommendation:** add unary minus to the unary tier, e.g.
`expression_unary = ("not" | "-") expression_unary | ... ;` (or a dedicated
`operator_negation`), unless number-negation-via-`0 -` is a deliberate choice —
in which case document it.

### C3. `return` only at the very end of a block

`block = "{" {statement} [return_statement] "}"` (line 274) allows a single
`return` and only as the final element, because `return_statement` is not part
of `statement`. This makes early returns impossible:
`if (x) { return a } ... ` in the *middle* of a block does not parse (the inner
block is fine, but a `return` followed by more statements in the same block is
not).

**Recommendation:** if early return is intended, add `return_statement` to the
`statement` union and drop the trailing `[return_statement]` from `block`. If
"single trailing return" is deliberate, note it — it's an unusual constraint.

---

## Precedence concern

### P1. Logical `not` binds tighter than comparison/equality

`not` sits in `expression_unary` (line 152), the tightest tier, alongside `++`
and `--`. That makes:

- `not a == b` parse as `(not a) == b`
- `not a > b` parse as `(not a) > b`

which is almost certainly not what a reader expects (and `(not a) > b` is a
likely type error). In most languages a logical-not binds *looser* than
comparisons but tighter than `and`/`or` (precedence: `or` < `and` < `not` <
comparison). `++`/`--` legitimately belong at the tight unary tier; `not` does
not.

**Recommendation:** split logical `not` into its own tier between
`expression_logical_and` and `expression_equality`, e.g.
`expression_not = "not" expression_not | expression_equality ;`, and drop
`operator_comparison_unary` from `expression_unary`. Confirm the intended
precedence first — this is a design decision, not strictly a bug.

---

## Dead / unreferenced rules

### D1. Most `operator_*` groups are now unused

After the expression tiers were spelled out with literal terminals, these are no
longer referenced anywhere:

- `operator_arithmetic` (line 67)
- `operator_comparison_binary` (line 69)
- `operator_binary` (line 74)
- `operator_unary` (line 76)
- `operator` (line 78)

Only `operator_comparison_unary` (used in `expression_unary`) is live. The groups
now duplicate the operator terminals that the tiers list independently, so they
can silently drift out of sync with the real grammar.

**Recommendation:** either wire them back in (have the tiers reference the
groups instead of raw terminals) or mark them explicitly as a token-classification
reference only (a comment), so it's clear they don't drive parsing. Note that
they can't be reused *directly* as tier operators, since `operator_comparison_binary`
mixes four different precedence levels (`or`, `and`, equality, relational).

### D2. `char_white_space` / `char_inline_white_space` unused

Already flagged in-file (lines 19–23) as "possible for removal once finalized" —
noting for completeness.

---

## Minor / nitpicks

- **M1.** `literal_decimal = digits "." digits ["d"]` (line 60) requires digits
  on both sides, so `.5` and `5.` are invalid, and there is no exponent form
  (`1e10`). Fine if intended; worth stating.
- **M2.** Pre/post increment and decrement are restricted to a bare `identifier`
  (lines 117–123), so `a.b++` and `arr[0]++` don't parse. Consistent, just a
  limitation to be aware of.
- **M3.** `expression_is_set` takes only an `identifier` (line 115), so
  `isSet(a.b)` is not expressible.
- **M4.** `expression_postfix` (line 157) has an inherent shift/reduce-style
  ambiguity between `expression_primary {accessor}` and
  `expression_post_increment`/`_decrement` (both start with `identifier`);
  resolvable with one token of lookahead, but worth a parser note.
- **M5.** The grammar defines no comment/trivia syntax for the *Tula* language
  itself, nor how whitespace separates tokens. These are typically lexer
  concerns, but if source-level comments exist they should be specified
  somewhere.
- **M6.** `numeric_iteration_initialization` accepts only
  `variable_local_definition` (line 235) — no global definition and no `set` on
  an existing variable in a `for` initializer. Likely intentional.

---

## Things done well

- The precedence-tiered expression grammar is correct and free of left
  recursion; right-associativity of `^` is encoded structurally via
  right-recursion rather than left to the parser.
- Several semantic rules (keyword-gated assignment, call-statement-must-end-in-a-call,
  boolean-typed conditions, table-key typing) are enforced structurally or
  documented with clear intent about the compile-time-vs-runtime boundary.
- Consistent naming, blank-line grouping, and a syntax-notes legend make the
  document easy to follow.

---

## Priority order

1. **B1** — add `comparison_statement` to `statement` (conditionals are currently unusable).
2. **B2** — add a top-level `program` start rule.
3. **C1** — fix the string/char character class and add escapes.
4. **C2 / P1** — decide unary minus and `not` precedence (both design calls with real consequences).
5. **C3** — decide the early-return policy.
6. **D1** — resolve the now-dead `operator_*` groups.
