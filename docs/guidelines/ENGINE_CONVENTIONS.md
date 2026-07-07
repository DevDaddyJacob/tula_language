# Tula Engine — Implementation Conventions

This document captures the engineering conventions that the Tula engine is built
on: how memory is owned, how failures are handled, how source positions are
tracked, and how the executables are assembled from one codebase. These are
*developer* concerns — the high-level shape of the pipeline (scanner → parser →
code generator → VM) is described in the architecture overview; this document is
about how that pipeline is implemented in C99, and what to keep consistent when
extending it.

When in doubt, match the surrounding source.

---

## 1. Data ownership flows top-down

Every stage **owns** the resources of the stage it drives, in a single top-down
chain. Tearing down the outermost object frees the whole graph beneath it.

- The scanner owns its buffered reader and the token array it fills; destroying
  the scanner destroys both.
- The parser owns the scanner and the AST it builds.
- The backend owns the AST it consumes.

Objects handed *back* across a stage boundary (for example, a token pointer
returned to a caller) are **borrowed** — they remain valid only as long as the
producing stage is alive. Callers must not free them or retain them past the
owner's lifetime.

This discipline is what keeps manual memory management tractable in C99. When you
add a stage or a data structure, decide up front who owns it, give that owner a
`*_destroy` that releases everything it owns, and make sure the owner is itself
reachable from the teardown path.

---

## 2. Two kinds of failure, handled differently

The engine deliberately separates failures in the *program being processed* from
failures in the *engine itself*, and they must never be conflated.

**Errors in the user's program** — a stray character, a malformed literal, a
syntax error — are *expected*; source code has bugs. They are represented as
**data flowing through the pipeline**, not as control-flow failures:

- The scanner emits a `TOK_ERROR` token carrying a message and source position.
- The parser surfaces syntax errors the same way, as error nodes/tokens.

These never crash the engine. A batch driver stops at the first one, but the
mechanism is ordinary token/AST flow. The payoff is honest diagnostics: an error
surfaced as a token or AST node always means "the source was wrong here," never
"the engine broke."

**Errors in the engine itself** — out of memory, an impossible internal state —
are *not* recoverable and are never represented as program data. They route
through the central exit helpers in `src/common/exit.h`:

- `tula_exit_err_no_mem()` — allocation failure.
- `tula_exit_err_internal(msg)` — an internal invariant was violated.
- `tula_exit(code)` / `tula_exit_error(...)` — the general forms.

These guarantee consistent cleanup and exit codes (`TULA_EXIT_*`). The project
rule is absolute:

> **Never call `exit()` directly, and never `return` from `main`.** Always go
> through the `tula_exit*` helpers so teardown and exit codes stay consistent.

Use the `UNREACHABLE_RETURN` / `NO_RETURN` markers (from `src/util.h`) after a
`NO_RETURN` exit helper so the compiler understands control does not continue.

---

## 3. Source positions are tracked end to end

A source location must be attached as early as possible and preserved through
every later representation, so that any diagnostic can point back to the exact
place in the original source.

- The buffered reader maintains the current `lineNumber` / `columnNumber` as
  characters are consumed.
- The scanner stamps each token with the line/column captured at the **start** of
  the lexeme.
- The AST, and later the bytecode, must carry positions forward too.

When you introduce a new representation, thread the position through it. Losing
position information at a stage boundary is a defect, not a detail — it degrades
every error message downstream of that point.

---

## 4. Growable, streaming data structures

The engine avoids fixed upper bounds wherever the size is not known in advance.

- Collections that grow (such as the token array) use the shared growth helpers
  and double their capacity on demand, governed by `TULA_ARRAY_GROW_FACTOR` and
  `TULA_ARRAY_MIN_THRESHOLD` in `src/config.h`. Do not hand-roll growth logic;
  reuse the array helpers.
- Input is read in fixed-size chunks (`TULA_READER_BUFFER_SIZE`), never by slurping
  the whole file — the engine is streaming and pull-based from the bottom.

Where a bound genuinely exists it is an explicit, configurable constant (for
example the maximum lengths for identifiers and string literals) rather than a
magic number buried in a function. New tunables belong in `src/config.h` alongside
the existing ones.

---

## 5. One codebase, many executables

Tula ships several executables, all built from the **same sources** and selected
at **compile time** — there is no runtime "mode" branch in the shipped binaries.
Each executable compiles in only the slice of the pipeline it needs:

| Executable | Carries | Purpose |
|------------|---------|---------|
| `tula` | frontend + code generator + VM | Full toolchain: compile to `.tbin`, run a `.tbin`, or run source directly. |
| `tulac` | frontend + code generator | Compiler: reads `.tula`, writes `.tbin`. |
| `tular` | VM only | Lean runner: executes `.tbin` files. |
| `tulad` | same as `tula`, debug mode | Debug build of the full toolchain, with extra checks and stage-level test entry points. Backs the automated testing framework (driving individual stages in isolation, e.g. printing a token stream) and is the build used during development. |

The selection is driven by the `TULA_EXECUTABLE_TYPE` compile definition in
`src/config.h`, which in turn defines the mode macros (e.g. `TULA_EXE_STANDARD`,
`TULA_EXE_DEBUGGING`). Large portions of the codebase — CLI options, config-struct
fields, test entry points — are conditionally compiled on these macros.

> **When editing anything guarded by a mode macro, keep every branch consistent.**
> A field added to the config struct under one mode, an option parsed under
> another, or a new executable variant must be reflected in each affected branch,
> or one build will break while another compiles clean.

New executable variants follow the same pattern: add the compile-time selection,
guard the variant-specific code behind its macro, and wire it to the appropriate
slice of the pipeline.

---

## 6. Process lifecycle

Every executable follows the same lifecycle, centralized so that setup and
teardown cannot drift between variants:

```
setup_global_state(argc, argv)   →   do the work for this build   →   teardown_global_state()   →   tula_exit(...)
```

- **Global state.** Process-wide configuration (raw `argc`/`argv` and the parsed
  CLI config) lives in a single `global_state_t`, reached through
  `get_global_state()`. Accessing it before `setup_global_state` has run is a
  fatal, defined error (`TULA_EXIT_ACCESS_STATE_BEFORE_INIT`) — not undefined
  behavior.
- **Stages stay pure with respect to global state.** The engine stages operate on
  the objects passed into them, not on globals. This is what lets a single stage
  run standalone under `tulad`, and lets the same stage code serve every product
  executable. Keep it that way: pass dependencies in, don't reach for
  `get_global_state()` from inside a stage.
- **Exit through the helpers.** As in §2, the lifecycle ends by calling
  `tula_exit`, never by returning from `main`, so cleanup always runs.
