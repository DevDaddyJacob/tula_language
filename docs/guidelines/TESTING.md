# Tula Testing Guide

Tula ships a small, two-tier test harness built on top of
[**Unity**](https://github.com/ThrowTheSwitch/Unity) (a C test framework,
vendored as a git submodule under `vendor/unity`) and driven by **CTest**.

There are two kinds of tests:

| Type | Location | What it exercises | How |
|------|----------|-------------------|-----|
| **Unit tests** | `test/unit_tests/` | Individual modules of `tula_lib`, in-process | Call functions directly and assert on return values |
| **Integration tests** | `test/integration_tests/` | The built `tulad` binary, end-to-end | Shell out to the binary, capture stdout, diff against a golden file |

Both register themselves with CTest, so a single `ctest` invocation runs
everything.

---

## Running the tests

```sh
# Configure + build (Unity submodule must be present:
#   git submodule update --init)
cmake -B build -S .
cmake --build build

# Run every test (unit + integration)
ctest --test-dir build --output-on-failure

# Run one suite by name (CTest name == the CMake target name)
ctest --test-dir build -R unit_test_token --output-on-failure

# Or run a test executable directly
build/test/unit_tests/unit_test_token
build/test/integration_tests/it_scanner_tokens
```

The CMake `all_tests`, `unit_tests`, and `integration_tests` custom targets are
convenience wrappers that just invoke `ctest --output-on-failure` over their
dependencies.

---

## Common ground: how a Unity test file is shaped

Every test executable is a standalone program with its own `main`. The Unity
contract is:

- Define `void setUp(void)` and `void tearDown(void)` — run before/after **each**
  test function (may be empty).
- Write test functions of the form `void test_xxx(void)` using `TEST_ASSERT_*`
  macros.
- In `main`, bracket the run with `UNITY_BEGIN()` / `UNITY_END()` and list each
  test with `RUN_TEST(...)`. `UNITY_END()`'s return value becomes the process
  exit code (non-zero = failure), which is what CTest keys off of.

```c
#include "unity.h"

void setUp(void) { }
void tearDown(void) { }

void test_example(void) {
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_example);
    return UNITY_END();
}
```

Unity behavior is configured per test tree by a local `unity_config.h`
(`test/unit_tests/unity_config.h`, `test/integration_tests/unity_config.h`);
both enable `UNITY_INCLUDE_EXEC_TIME`. The **Unity memory extension** is turned
on globally (`set(UNITY_EXTENSION_MEMORY ON ...)` in the root `CMakeLists.txt`),
which gives unit tests optional heap-leak detection (see below).

---

## 1. Unit tests

### How they're implemented

Unit tests link **directly against `tula_lib`** (the static library holding all
of `src/`) plus `unity`, with `src/` on the include path. They call library
functions in-process and assert on the results — no subprocess, no I/O contract.

Registration is driven by the `unit_test()` helper in
`test/unit_tests/CMakeLists.txt`:

```cmake
function(unit_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE tula_lib unity)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

unit_test(unit_test_buffered_reader test_buffered_reader.c)
unit_test(unit_test_strings         test_strings.c)
unit_test(unit_test_token           test_token.c)
```

Each call produces both a build target and a CTest test of the same name, and
the target must also be listed in the `unit_tests` custom target's `DEPENDS`.

Two patterns worth knowing from the existing suites:

- **Table-driven assertions via the X-macro tables.** `test_token.c` reuses the
  `DEFINE_TOKENS` X-macro to assert every token's parallel-array entry in a few
  lines, so the test can never drift out of sync with the token table:
  ```c
  void test_token_value_matches(void) {
  #define DEFINER(identifier, value, _2, _3, _4, _5, _6) \
      TEST_ASSERT_TRUE(str_equals(TOKENS_VALUE[identifier], value, strlen(value)));
      DEFINE_TOKENS(DEFINER)
  #undef DEFINER
  }
  ```
- **Leak detection with the Unity memory extension.** `test_buffered_reader.c`
  brackets each test with `UnityMalloc_StartTest()` / `UnityMalloc_EndTest()`
  (from `unity_memory.h`) in `setUp`/`tearDown` to catch unbalanced
  `malloc`/`free`. Local fixtures (temp files) are also set up and cleaned up
  there.

### Example: adding a new unit test

Say we want to test `str_starts_with_char` from `src/common/strings.c`.

**1. Create `test/unit_tests/test_my_strings.c`:**

```c
#include "unity.h"
#include "common/strings.h"   /* included relative to src/ */

void setUp(void) { }
void tearDown(void) { }

void test_starts_with_char_true(void) {
    TEST_ASSERT_TRUE(str_starts_with_char("apple", 'a'));
}

void test_starts_with_char_false(void) {
    TEST_ASSERT_FALSE(str_starts_with_char("apple", 'z'));
}

void test_starts_with_char_null_is_false(void) {
    TEST_ASSERT_FALSE(str_starts_with_char(NULL, 'a'));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_starts_with_char_true);
    RUN_TEST(test_starts_with_char_false);
    RUN_TEST(test_starts_with_char_null_is_false);
    return UNITY_END();
}
```

**2. Register it in `test/unit_tests/CMakeLists.txt`** — add the `unit_test(...)`
call and list the target under the `unit_tests` custom target:

```cmake
unit_test(unit_test_my_strings test_my_strings.c)

add_custom_target(unit_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
        unit_test_buffered_reader
        unit_test_strings
        unit_test_token
        unit_test_my_strings)   # <-- added
```

**3. Reconfigure and run:**

```sh
cmake -B build -S .
ctest --test-dir build -R unit_test_my_strings --output-on-failure
```

---

## 2. Integration tests

### How they're implemented

Integration tests are **black-box, golden-file tests**. They do not link the
library; instead they run the compiled **`tulad`** debug binary as a subprocess,
capture its stdout, and compare it byte-for-byte against a recorded expected
output file.

The plumbing lives in `integration_test_lib`
(`integration_test_helper.{h,c}`), which provides:

- `dynamic_buffer_t` — a small growable byte buffer.
- `read_file(path, &buf)` — slurp a `.out` fixture into a buffer.
- `execute_tulad(args, &stdoutBuf, &status)` — build the command line
  `"<tulad path> <args>"`, run it via `popen`/`_popen` (wrapped as `os_popen`
  for cross-platform use), and capture stdout + exit status.
- `EXECUTE_TULAD_EXPECT_OUTPUT(expectedFile, args)` — the assertion macro: reads
  the expected file, runs `tulad`, asserts exit status `0`, and asserts the
  captured stdout equals the expected contents.

Two compile definitions wire the tests to the real artifacts (set in
`test/integration_tests/CMakeLists.txt`):

- `TULAD_EXE_PATH` — the absolute path of the just-built `tulad`
  (`$<TARGET_FILE:tulad>`).
- `INTEGRATION_TEST_DATA_DIR` — the absolute path of the `data/` fixtures
  directory.

Registration uses the `integration_test()` helper, which additionally forces the
`tulad` binary to be built first (nothing else creates that dependency):

```cmake
function(integration_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE integration_test_lib unity)
    target_compile_definitions(${test_name} PRIVATE
            TULAD_EXE_PATH="$<TARGET_FILE:tulad>"
            INTEGRATION_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_dependencies(${test_name} tulad)   # build the binary under test first
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

integration_test(it_scanner_tokens it_scanner_tokens.c)
```

The current suite, `it_scanner_tokens.c`, tests the scanner via `tulad`'s
`--mode scanner` debug mode. It uses a macro so each fixture pair becomes one
test:

```c
#define DATA_DIR INTEGRATION_TEST_DATA_DIR "/it_scanner/"

#define DEFINE_TULAD_TEST_SCANNER(name) \
    void test_scanner_##name(void) { \
        EXECUTE_TULAD_EXPECT_OUTPUT( \
            DATA_DIR #name ".out", \
            "--mode scanner --input-file \"" DATA_DIR #name ".in\""); \
    }

DEFINE_TULAD_TEST_SCANNER(all_tokens)
DEFINE_TULAD_TEST_SCANNER(sample_1)
/* ... */
```

### Fixtures: `.in` / `.out` pairs

Fixtures live in `test/integration_tests/data/<suite>/`. Each test is a pair:

- `<name>.in` — the Tula source fed to `tulad` (`--input-file`).
- `<name>.out` — the exact stdout `tulad` is expected to produce.

For the scanner suite, `.in` is Tula source and `.out` is the printed token
stream, e.g.:

```
# all_tokens.in                  # all_tokens.out (excerpt)
and                              <token_t> {type: "TOK_AND", line: 7, column: 1, content: "and", contentLength: 3}
break                            <token_t> {type: "TOK_BREAK", line: 8, column: 1, content: "break", contentLength: 5}
```

### Example: adding a new integration test

The easiest case is adding another scanner fixture to the existing suite.

**1. Create the input** `test/integration_tests/data/it_scanner/sample_10.in`:

```tula
def var greeting = "hello"
```

**2. Generate the expected output.** Run `tulad` by hand and capture its stdout
into the `.out` file, then eyeball it to confirm it's correct before trusting
it:

```sh
build/tula.exe        # (the standard build; or use tulad directly:)
build/tulad --mode scanner --input-file \
    test/integration_tests/data/it_scanner/sample_10.in \
    > test/integration_tests/data/it_scanner/sample_10.out
```

**3. Wire the test up in `it_scanner_tokens.c`** — add the definition and a
matching `RUN_TEST`:

```c
DEFINE_TULAD_TEST_SCANNER(sample_10)      /* with the other definitions */

int main(void) {
    UNITY_BEGIN();
    /* ... existing RUN_TEST calls ... */
    RUN_TEST(test_scanner_sample_10);     /* <-- added */
    return UNITY_END();
}
```

**4. Rebuild and run:**

```sh
cmake --build build
ctest --test-dir build -R it_scanner_tokens --output-on-failure
```

#### Adding a whole new integration suite

To test a different `tulad` mode, create `it_<thing>.c` next to the existing
suite, add a `data/it_<thing>/` fixtures directory, register it with
`integration_test(it_<thing> it_<thing>.c)`, and add the target to the
`integration_tests` custom target's `DEPENDS`. Reuse
`EXECUTE_TULAD_EXPECT_OUTPUT` for the golden-file comparison.

---

## Choosing which type to write

- Reach for a **unit test** when you can assert on a function's return value or
  on a struct's state directly — it's faster, pinpoints failures to a single
  module, and can check for memory leaks.
- Reach for an **integration test** when the behavior is only observable through
  the program's output (e.g. the full scan of a source file), or when you want
  to lock in end-to-end behavior against regressions. Because they depend on
  `tulad`'s debug print format, integration `.out` fixtures must be regenerated
  whenever that output format intentionally changes.
