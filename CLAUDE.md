# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**Tula** is a custom programming language interpreter written in **C99** and built with **CMake**. It is in early development — the scanner (lexer) is functional; the parser and AST (`src/engine/parser/`) are stubs. The language design reference lives in `docs/EBNF.md` and `docs/demo-v2.tula` (an annotated feature tour).

## Build & test

The build supports three toolchains, selected automatically: **MSVC** on Windows (required — a non-MSVC compiler on Windows is a hard `FATAL_ERROR`), **GCC** on Linux, **Clang** on macOS. CLion is the intended IDE; existing build dirs are `cmake-build-msvc-debug/` and `cmake-build-msvc-release/`.

From a fresh checkout (note: Unity is a git submodule — `git submodule update --init` if `vendor/unity` is empty):

```sh
cmake -B build -S .
cmake --build build

# Run all tests (unit + integration) via CTest
ctest --test-dir build --output-on-failure

# Run a single test executable directly, e.g.
build/test/unit_tests/unit_test_token
build/test/integration_tests/it_scanner_tokens
```

The `all_tests`, `unit_tests`, and `integration_tests` custom CMake targets each just invoke `ctest --output-on-failure` over their dependencies.

## Two executables: `tula` vs `tulad`

Both build from the same `src/tula.c` but with different `TULA_EXECUTABLE_TYPE` compile definitions (see `src/config.h`):

- **`tula`** (`TULA_EXECUTABLE_TYPE=0` → `TULA_EXE_STANDARD`) — the real CLI. Compiled with warnings-as-errors (`/WX` / `-Werror`).
- **`tulad`** (`TULA_EXECUTABLE_TYPE=1` → `TULA_EXE_DEBUGGING`) — a debugging harness. Instead of running programs it dispatches to test modes (`--mode scanner --input-file <path>`) defined in `src/test.c`, printing internal state (e.g. token streams) to stdout.

Large swaths of the codebase are conditionally compiled on `TULA_EXE_STANDARD` vs `TULA_EXE_DEBUGGING` (CLI options, config struct fields, etc.). When editing `src/state/cli.c`, `src/state/cli.h`, or `src/test.*`, keep **both** branches consistent.

## Architecture

- **Entry / lifecycle** — `src/tula.c` `main` calls `setup_global_state` → work → `teardown_global_state` → `tula_exit`. Never `return` from `main` or call `exit()` directly; use `tula_exit` / the `tula_exit_err_*` helpers in `src/common/exit.h` so cleanup and exit codes stay consistent.
- **Global state** — `src/state/gstate.c` holds a single process-wide `global_state_t` (argc/argv + parsed `cli_config_t`). Access via `get_global_state()`; accessing before `setup_global_state` exits with `TULA_EXIT_ACCESS_STATE_BEFORE_INIT`.
- **CLI parsing** — `src/state/cli.c` hand-rolls option parsing into `cli_config_t`.
- **Engine pipeline** — `src/engine/`:
  - `scanner/token.*` — token definitions and the dynamic `arr_token_t` array.
  - `scanner/scanner.*` — pulls characters from a `buf_reader_t` and produces tokens (`scanner_read_next` / `scanner_read_all`).
  - `parser/` — **not yet implemented** (empty headers).
- **Common utilities** — `src/common/`: `buffered_reader.*` (chunked file reader, buffer size `TULA_READER_BUFFER_SIZE`), `strings.*` (safe string ops like `str_copy_safe`, `str_equals_partial`), `os.*` (platform detection macros), `trace.*` (stack traces — DbgHelp on Windows, `backtrace()` on Unix), `exit.*`.
- **Tunables** — `src/config.h` centralizes constants: array growth (`TULA_ARRAY_GROW_FACTOR`, `TULA_ARRAY_MIN_THRESHOLD`), buffer sizes, trace limits, and the OS/compiler-detection macros.

### The X-macro pattern (important)

Token and test-mode tables are defined once as an X-macro list and expanded multiple ways. To **add a token**, edit the `DEFINE_TOKENS(def)` list in `src/engine/scanner/token.h` (each row: identifier, value, alt-value, isMeta, isPrimitive, isKeyword, isOperator) — the enum and every parallel `TOKENS_*` lookup array regenerate automatically. Same pattern for `DEFINE_TEST_MODES` in `src/test.h`. Do not hand-maintain the parallel arrays.

## Tests

- **Unit tests** (`test/unit_tests/`) link against `tula_lib` + Unity and test modules directly. Register a new one by adding a `unit_test(<name> <source>.c)` call and listing it in the `unit_tests` target's `DEPENDS` in `test/unit_tests/CMakeLists.txt`.
- **Integration tests** (`test/integration_tests/`) are golden-file tests: they shell out to the built `tulad` binary via `execute_tulad`, capture stdout, and compare against a `.out` fixture (see `EXECUTE_TULAD_EXPECT_OUTPUT`). Fixtures live in `test/integration_tests/data/<suite>/<name>.in` / `.out`. `it_scanner_tokens.c` uses the `DEFINE_TULAD_TEST_SCANNER` macro — adding a `sample_N` case means adding the `.in`/`.out` files, a `DEFINE_TULAD_TEST_SCANNER(sample_N)` line, and a matching `RUN_TEST` in `main`.

## Conventions

- **Formatting**: `.clang-format` (LLVM-based). Hard tabs, tab width 4, 120-col limit, braces on their own line (Allman-style) for functions/control/case labels, case labels indented.
- **Yoda conditions** are used throughout (`NULL == ptr`, `2 > strlen(arg)`); match the surrounding style.
- Source files follow a banner-comment layout (`Macros`, `Typedefs & Prototypes`, `Module Level Variables & Constants`, `Function Definitions`). File-local helpers are `static` with prototypes declared up top.
- Use the `UNREACHABLE_*`, `UNUSED`, `NO_RETURN` macros from `src/util.h` rather than compiler-specific builtins directly.
- Vendored code under `vendor/` (Unity) is not ours — its warnings are deliberately silenced in the root `CMakeLists.txt`; don't edit vendored files.
