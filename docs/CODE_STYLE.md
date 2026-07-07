# Tula Code Style Guide

This document describes the C coding conventions used throughout the Tula
codebase. It is descriptive of the existing code — when in doubt, match the
surrounding source. New code should read like it was written by the same hand
as the rest of the project.

The language target is **C99**. Code must compile clean under MSVC (`/W4`, and
`/WX` for the `tula` target), GCC, and Clang (`-Wall -Wextra`).

---

## 1. Formatting

Formatting is enforced by [`.clang-format`](../.clang-format) (LLVM base). The
key rules:

- **Indentation: hard tabs**, tab width 4. Never spaces for indentation.
- **Column limit: 120.**
- **Braces: Allman style** — the opening brace goes on its own line for
  functions, `struct`/`enum`/`union`, and control statements.
  ```c
  void token_destroy(token_t* token)
  {
      if (NULL == token)
      {
          return;
      }
  }
  ```
- **`case` labels are indented** one level inside `switch`, and non-trivial case
  bodies are wrapped in their own brace block:
  ```c
  switch (optionType)
  {
      case OPTION_INVALID:
      {
          exit_err_bad_usage_f("'%s' is not a recognized option.", arg);
      }

      case OPTION_HELP:
      {
          print_help_menu();
          break;
      }
  }
  ```
- **Pointers bind left**: `char* content`, not `char *content`.
- **Up to two consecutive blank lines** are permitted (and are used liberally to
  separate logical steps inside a function body — see §7).
- Files end with a trailing newline.

---

## 2. Naming

| Kind | Convention | Example |
|------|-----------|---------|
| Functions | `lower_snake_case`, module-prefixed | `buf_reader_read`, `str_copy_safe` |
| Local variables & parameters | `lowerCamelCase` | `contentLength`, `destSize`, `oldCapacity` |
| Struct / enum types (typedef'd) | `lower_snake_case` with `_t` suffix | `token_t`, `cli_config_t`, `arr_token_t` |
| Underlying `struct`/`enum` tag | `tula_`-prefixed | `struct tula_token`, `enum tula_token_type` |
| Enum members | `UPPER_SNAKE_CASE`, group-prefixed | `TOK_IDENT`, `OPTION_HELP`, `TEST_MODE_SCANNER` |
| Macros & compile-time constants | `UPPER_SNAKE_CASE` | `TULA_MAX_BUFFER_SIZE`, `UNREACHABLE_HINT` |
| Global lookup tables | `UPPER_SNAKE_CASE` | `TOKENS_IS_KEYWORD`, `TEST_MODE_VALUE` |

Note the deliberate split: **types and functions are snake_case, but variables
are camelCase.** Keep it consistent.

Module functions are prefixed with a short module name that matches the file:
`token_*`, `arr_token_*`, `buf_reader_*`, `str_*`, `cli_*`, `scanner_*`,
`tula_exit_*`.

### Constructor / destructor pattern

Heap-managed types follow a uniform lifecycle naming:

- `xxx_new(...)` — allocates and returns a pointer (or `NULL` on allocation
  failure).
- `xxx_destroy(xxx* )` — frees the object and its owned members; a `NULL`
  argument is a safe no-op.
- `xxx_init(xxx* )` — zero-initializes a caller-owned struct in place.

See `token_new` / `token_destroy` / `arr_token_init` for the reference shape.

---

## 3. Header files

- **Include guards**, not `#pragma once`. The macro name mirrors the path from
  `src/`: `src/common/strings.h` → `TULA_COMMON_STRINGS_H`,
  `src/engine/scanner/token.h` → `TULA_ENGINE_SCANNER_TOKEN_H`.
  ```c
  #ifndef TULA_COMMON_STRINGS_H
  #define TULA_COMMON_STRINGS_H
  /* ... */
  #endif /* TULA_COMMON_STRINGS_H */
  ```
  The closing `#endif` carries the guard name as a trailing comment.
- Public API declarations live in the header with **Doxygen-style doc comments**
  (see §6). Prototypes are separated by a blank line each.
- Keep headers minimal: include only what the header itself needs
  (`<stdbool.h>`, `<stdint.h>`, etc.).

## 4. Include ordering

Includes are grouped, matching `.clang-format`'s three-priority scheme, with a
blank line between groups:

1. The module's own header first (in a `.c` file), e.g. `#include "token.h"`.
2. C standard library headers: `<stdio.h>`, `<stdlib.h>`, `<string.h>`.
3. Project headers by path: `"util.h"`, `"common/strings.h"`.

```c
#include "token.h"

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "common/strings.h"
```

Project headers are included relative to `src/` (which is on the include path),
e.g. `#include "common/exit.h"` — not `../../common/exit.h`.

---

## 5. Source file layout

Every `.c` (and larger `.h`) file is organized into the same banner-delimited
sections, in this order. Empty sections are kept as placeholders (often with a
commented-out example) so the skeleton is always present:

```c
/*
 * ==================================================
 * Macros
 * ==================================================
 */

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */
```

File-local helpers are declared `static` with a prototype in the
**Typedefs & Prototypes** section, then defined in order in
**Function Definitions**. Section headers within `config.h`/`util.h` use a wider
`====` banner style for top-level configuration blocks — follow whichever style
already exists in the file you are editing.

---

## 6. Comments & documentation

- **Public functions** get a Doxygen block in the header using `\brief`,
  `\param[in]` / `\param[out]`, `\return`, and `\note`:
  ```c
  /**
   * \brief           Safely copies a string into a destination buffer, at most
   *                  copying size - 1 characters and always ensuring a null
   *                  termination
   * \param[out]      destination: The destination buffer to write into
   * \param[in]       source: The source to copy into the destination
   * \param[in]       destSize: The size of the destination buffer
   * \return          Returns the total number of characters copied including
   *                  the null terminator
   */
  ```
  Descriptions are aligned in a column after the tag.
- **Struct fields** are documented inline with `/** \brief ... */` blocks.
- **Inside function bodies**, use `/* ... */` block comments to narrate each
  step ("Null check our arguments", "Copy the characters up to destSize - 1").
  This step-by-step commenting is a strong house style.
- `//` line comments are used sparingly, mostly for IDE pragmas
  (`// ReSharper disable ...`) and in `.tula` sample/design files.
- Unimplemented functions are marked with a `TBD` doc comment rather than left
  undocumented.

---

## 7. Function bodies

- **Guard clauses first.** Validate arguments (especially `NULL` checks and
  length/size checks) and return early before doing real work:
  ```c
  bool str_starts_with_char(const char* str, const char target)
  {
      if (str == NULL)
      {
          return false;
      }

      return str[0] == target;
  }
  ```
- Separate each logical stage with a blank line (often two) and a short
  narrating comment.
- Prefer `const` on parameters and locals that don't change — even by-value
  scalar parameters are marked `const` in definitions
  (`const size_t destSize`, `const uint32_t line`).
- Declare loop variables in the `for` statement (`for (size_t i = 0; ...)`);
  C99 mixed declarations are fine.

---

## 8. Conditionals: Yoda style

Comparisons against constants and `NULL` put the **constant on the left**. This
is pervasive and intentional (it defends against accidental `=`):

```c
if (NULL == token)          /* not: token == NULL */
if (0 >= destSize)          /* not: destSize <= 0 */
if ('-' != arg[0] || 2 > strlen(arg))
```

Match this in new code. (A few older spots use `str == NULL`; the Yoda form is
the preferred direction going forward.)

---

## 9. Memory & error handling

- Check every `malloc` result. On failure inside a `_new` function, free any
  partial allocations and return `NULL`:
  ```c
  token->content = malloc(sizeof(char) * (contentLength + 1));
  if (NULL == token->content)
  {
      free(token);
      return NULL;
  }
  ```
- Use the array helpers from `util.h` for growable buffers rather than
  open-coding `realloc`: `tula_array_grow_capacity`, `tula_array_resize`,
  `tula_array_free` (all routed through `tula_array_reallocate`).
- **Never call `exit()` or `return` from `main` directly.** Terminate through
  the `src/common/exit.h` API so exit codes and cleanup stay consistent:
  - `tula_exit(TULA_EXIT_GOOD)` for normal termination.
  - The `tula_exit_err_*` helpers and `exit_err_*_f` macros for error paths,
    which map to the `TULA_EXIT_*` codes.
  - Fatal-exit functions are annotated `NO_RETURN`.
- Free before exiting on error paths that own heap memory (see `cli.c`, which
  frees `params`/`config` before calling `tula_exit_err_*`).
- Destructors are `NULL`-tolerant no-ops so they can be called unconditionally.

---

## 10. Portability

- OS and compiler detection is centralized in `src/common/os.h` and
  `src/config.h`. Use the resulting macros — `OS_WINDOWS`, `OS_POSIX_COMPLIANT`,
  `COMPILER_MSVC`, `COMPILER_GCC`, `COMPILER_CLANG` — rather than raw
  `_WIN32` / `__GNUC__` checks in feature code.
- Wrap compiler-specific intrinsics behind the `util.h` macros
  (`UNREACHABLE_HINT`, `UNREACHABLE_RETURN`, `UNREACHABLE_DEFAULT`, `NO_RETURN`,
  `UNUSED`) instead of using `__builtin_unreachable` / `__assume` directly.
- Use fixed-width integer types from `<stdint.h>` (`int32_t`, `uint32_t`,
  `size_t`) rather than bare `int`/`long` for anything size- or
  width-sensitive.

---

## 11. The X-macro table pattern

Enumerations that need parallel data (string names, boolean flags) are defined
once as an **X-macro list** and expanded multiple ways. This is the canonical
pattern for `DEFINE_TOKENS` (`token.h`) and `DEFINE_TEST_MODES` (`test.h`).

To extend such a table, **edit the `DEFINE_*` list only** — the enum and every
parallel `[identifier] = value` lookup array regenerate automatically via
designated-initializer definer macros. Never hand-maintain the derived arrays.

```c
#define DEFINE_TOKENS(def)                                       \
    def(TOK_IDENT, "<identifier>", NULL, false, false, false, false) \
    /* ... */

/* enum members */
#define TOKEN_ENUM_DEFINER(identifier, ...) identifier,
typedef enum tula_token_type { DEFINE_TOKENS(TOKEN_ENUM_DEFINER) TOTAL_TOKENS } token_type_t;

/* parallel lookup table */
#define TOKEN_VALUE_DEFINER(identifier, value, ...) [identifier] = value,
const char* TOKENS_VALUE[TOTAL_TOKENS] = { DEFINE_TOKENS(TOKEN_VALUE_DEFINER) };
```

Each such enum ends with a `TOTAL_*` sentinel used to size the arrays.

---

## 12. Conditional compilation

The same sources build two executables via `TULA_EXECUTABLE_TYPE`
(see `config.h`): `TULA_EXE_STANDARD` (`tula`) and `TULA_EXE_DEBUGGING`
(`tulad`). Debug-only declarations, struct fields, and functions are gated:

```c
#ifdef TULA_EXE_DEBUGGING
void token_print(const token_t* token);
#endif /* TULA_EXE_DEBUGGING */
```

- Always close a non-trivial `#if`/`#ifdef` with a trailing comment naming the
  condition: `#endif /* TULA_EXE_DEBUGGING */`.
- When you touch one mode's branch (in `cli.c`, `cli.h`, `test.*`), check
  whether the other mode needs a matching change.
- Vendored third-party code (`vendor/`) is exempt from this guide; its warnings
  are silenced in CMake and it is not edited to fit house style.
