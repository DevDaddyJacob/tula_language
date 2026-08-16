# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## What this is

**Tula** is a custom programming language, written in **C99** and built with
**CMake**, that is at once a compiler and an interpreter — see
`docs/architecture/DESIGN.md` for the full pipeline. It is in early
development: the reader and scanner (lexer) are functional, and the parser
(`src/engine/parser/`) now produces a real AST for a growing subset of the
grammar (variables, functions, control flow, accessors, collections — see
`test/integration_tests/data/it_parser/`). The code generator
(`src/engine/codegen/`) currently has only opcode definitions
(`opcode.h`/`.c`); the VM does not exist yet. The language design reference
lives in `docs/language/EBNF.md`, `docs/language/BYTECODE.md` (the `.tbin`
instruction set), and `docs/language/demo-v1.tula` / `demo-v2.tula`
(annotated feature tours). `docs/language/VERSIONS.md` tracks the feature
roadmap across language versions.

Read `docs/DOCUMENTATION.md` for how the `docs/` tree itself is organized
(`architecture/`, `guidelines/`, `language/`).

## Build & test

The build supports three toolchains, selected automatically: **MSVC** on
Windows (required — a non-MSVC compiler on Windows is a hard
`FATAL_ERROR`), **GCC** on Linux, **Clang** on macOS. CLion is the intended
IDE; existing build dirs are `cmake-build-msvc-debug/` and
`cmake-build-msvc-release/`.

From a fresh checkout (note: Unity is a git submodule —
`git submodule update --init` if `vendor/unity` is empty):

```sh
cmake -B build -S .
cmake --build build

# Run all tests (unit + integration) via CTest
ctest --test-dir build --output-on-failure

# Run a single test executable directly, e.g.
build/test/unit_tests/unit_test_token
build/test/integration_tests/it_scanner_tokens
```

The `all_tests`, `unit_tests`, and `integration_tests` custom CMake targets
each just invoke `ctest --output-on-failure` over their dependencies.

## The executables: `tula`, `tulac`, `tular`, `tulad`

Tula's pipeline (`reader → scanner → parser → codegen → VM`, see
`docs/architecture/DESIGN.md`) has two seams: **after the AST** (frontend
vs. backend) and **at the `.tbin` bytecode** (codegen vs. VM). Those seams
are what let one shared frontend fork into several executables, each
carrying only the slice of the pipeline it needs. Only `tula` and `tulad`
exist today; `tulac`/`tular` are the designed end state once codegen and the
VM exist.

| Executable | Carries | Purpose |
|------------|---------|---------|
| **`tula`** | frontend + codegen + VM | The full toolchain: compile `.tula` → `.tbin`, run a `.tbin`, or run `.tula` source directly in memory. |
| **`tulac`** *(planned)* | frontend + codegen | Compiler only: reads `.tula`, writes `.tbin`, never executes. |
| **`tular`** *(planned)* | VM only | Lean runner: loads a `.tbin` and executes it; carries no frontend. |
| **`tulad`** | same as `tula`, debug build | Dispatches to test modes (`--mode scanner --input-file <path>`) defined in `src/test.c`, printing internal state (token streams, AST) to stdout. Backs the integration test suite. |

All build from the same `src/tula.c`, selected at compile time via the
`TULA_EXECUTABLE_TYPE` definition (see `src/config.h`), which resolves to
mode macros like `TULA_EXE_STANDARD` / `TULA_EXE_DEBUGGING`. `tula` is
compiled with warnings-as-errors (`/WX` / `-Werror`).

Large swaths of the codebase are conditionally compiled on these macros (CLI
options, config struct fields, etc.). When editing `src/state/cli.c`,
`src/state/cli.h`, or `src/test.*`, keep **every** branch consistent — see
`docs/guidelines/ENGINE_CONVENTIONS.md` §5.

## Architecture

For the full pipeline design (why it's staged, streaming, and pull-based;
how the three execution methods — compile, run-compiled, run-directly —
share one frontend and fork the backend) see `docs/architecture/DESIGN.md`,
with one document per stage: `READER.md`, `SCANNER.md`, `PARSER.md`,
`CODEGEN.md`, `BYTECODE.md`, `VM.md`.

- **Entry / lifecycle** — `src/tula.c` `main` calls `setup_global_state` →
  work → `teardown_global_state` → `tula_exit`. Never `return` from `main`
  or call `exit()` directly; use `tula_exit` / the `tula_exit_err_*` helpers
  in `src/common/exit.h` so cleanup and exit codes stay consistent.
- **Global state** — `src/state/gstate.c` holds a single process-wide
  `global_state_t` (argc/argv + parsed `cli_config_t`). Access via
  `get_global_state()`; accessing before `setup_global_state` exits with
  `TULA_EXIT_ACCESS_STATE_BEFORE_INIT`. Engine stages themselves stay pure
  with respect to global state — pass dependencies in rather than calling
  `get_global_state()` from inside a stage.
- **CLI parsing** — `src/state/cli.c` hand-rolls option parsing into
  `cli_config_t`.
- **Engine pipeline** — `src/engine/`:
  - `scanner/token.*` — token definitions and the dynamic `arr_token_t`
    array.
  - `scanner/scanner.*` — pulls characters from a `buf_reader_t` and
    produces tokens (`scanner_read_next` / `scanner_read_all`).
  - `parser/ast.*` — the AST node types.
  - `parser/parser.*` — consumes the token stream and builds the AST;
    detects syntax errors as data (not crashes), matching the scanner's
    error-token convention. (`parser_bak.*` is retained scratch/backup
    code, not part of the active build surface — check before extending
    it.)
  - `codegen/opcode.*` — bytecode opcode definitions only so far; the
    AST-to-bytecode lowering pass and the VM itself are not yet
    implemented.
- **Common utilities** — `src/common/`: `buffered_reader.*` (chunked file
  reader, buffer size `TULA_READER_BUFFER_SIZE`, tracks line/column for
  positions), `strings.*` (safe string ops like `str_copy_safe`,
  `str_equals_partial`), `numeric.*` (numeric literal parsing/typing
  helpers), `os.*` (platform detection macros), `trace.*` (stack traces —
  DbgHelp on Windows, `backtrace()` on Unix), `exit.*`.
- **Tunables** — `src/config.h` centralizes constants: array growth
  (`TULA_ARRAY_GROW_FACTOR`, `TULA_ARRAY_MIN_THRESHOLD`), buffer sizes,
  trace limits, and the OS/compiler-detection macros.

### Cross-cutting conventions (see `docs/guidelines/ENGINE_CONVENTIONS.md`)

- **Ownership flows top-down**: each stage owns the resources of the stage
  it drives (parser owns its scanner and the AST it builds, etc.); objects
  handed back across a stage boundary are borrowed, not owned by the
  caller.
- **Two kinds of failure**: errors *in the program being processed* (bad
  syntax, a malformed literal) are data — an error token/AST node with a
  message and position, never a crash. Errors *in the engine itself* (OOM,
  broken invariants) are fatal and always routed through
  `tula_exit_err_*`/`tula_exit_error` — never a bare `exit()`.
- **Positions travel with the data**: line/column is captured at the reader
  and must be threaded through every later representation (tokens → AST →
  bytecode) so diagnostics can always point back to source.

### The X-macro pattern (important)

Token and test-mode tables are defined once as an X-macro list and expanded
multiple ways. To **add a token**, edit the `DEFINE_TOKENS(def)` list in
`src/engine/scanner/token.h` (each row: identifier, value, alt-value,
isMeta, isPrimitive, isKeyword, isOperator) — the enum and every parallel
`TOKENS_*` lookup array regenerate automatically. Same pattern for
`DEFINE_TEST_MODES` in `src/test.h`. Do not hand-maintain the parallel
arrays.

## Tests

See `docs/guidelines/TESTING.md` for the full guide (fixture format, how to
add a suite, when to reach for unit vs. integration tests).

- **Unit tests** (`test/unit_tests/`) link against `tula_lib` + Unity and
  test modules directly, in-process. Register a new one with a
  `unit_test(<name> <source>.c)` call in `test/unit_tests/CMakeLists.txt`,
  and list the target under the `unit_tests` custom target's `DEPENDS`.
- **Integration tests** (`test/integration_tests/`) are black-box
  golden-file tests: they shell out to the built `tulad` binary via
  `execute_tulad`, capture stdout, and compare against a `.out` fixture with
  the `EXECUTE_TULAD_EXPECT_OUTPUT` macro. Fixtures live in
  `test/integration_tests/data/<suite>/<name>.in` / `.out`. Two suites
  exist today: `it_scanner_tokens` (`--mode scanner`,
  `DEFINE_TULAD_TEST_SCANNER` macro, fixtures under `data/it_scanner/`) and
  `it_parser_ast` (`data/it_parser/`). Adding a fixture case means adding
  the `.in`/`.out` files, the suite's `DEFINE_TULAD_TEST_*` line, and a
  matching `RUN_TEST` in `main`. Register a brand-new suite with
  `integration_test(<name> <source>.c)` in
  `test/integration_tests/CMakeLists.txt`.

## Conventions

Full style guide: `docs/guidelines/CODE_STYLE.md`. Key points:

- **Formatting**: `.clang-format` (LLVM-based). Hard tabs, 80-col soft
  limit in `src/` (120 absolute), Allman-style braces (own line) for
  functions/control/case labels, case labels indented one level.
- **Comments**: `/* ... */` only — `//` is not allowed, even for
  single-line comments. Multi-line blocks use a leading `*` per line;
  double-`**` is reserved for Doxygen.
- **Yoda conditions** are used throughout (`NULL == ptr`,
  `2 > strlen(arg)`); match the surrounding style.
- **Naming**: functions and typedef'd struct/enum types are
  `lower_snake_case` (module-prefixed, e.g. `scanner_read_next`,
  `token_t`); locals/parameters are `lowerCamelCase`; enum members and
  macros are `UPPER_SNAKE_CASE`.
- Source files follow a banner-comment layout (`Macros`,
  `Typedefs & Prototypes`, `Module Level Variables & Constants`,
  `Function Definitions`), sections kept as placeholders even when empty.
  File-local helpers are `static` with prototypes declared up top.
- Header guards mirror the path from `src/` (e.g.
  `src/engine/scanner/token.h` → `TULA_ENGINE_SCANNER_TOKEN_H`), with the
  guard name repeated as a trailing comment on `#endif`.
- Use the `UNREACHABLE_*`, `UNUSED`, `NO_RETURN` macros from `src/util.h`
  rather than compiler-specific builtins directly.
- Growable buffers use the shared helpers (`tula_array_grow_capacity`,
  `tula_array_resize`, `tula_array_free`) rather than hand-rolled `realloc`.
- Heap-managed types follow `xxx_new`/`xxx_destroy`/`xxx_init` lifecycle
  naming (§7(c) of the style guide).
- Vendored code under `vendor/` (Unity) is not ours — its warnings are
  deliberately silenced in the root `CMakeLists.txt`; don't edit vendored
  files.
