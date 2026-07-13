# Tula Code Style Guide
This document describes the C coding conventions used throughout the Tula
codebase. New code should follow guidelines outlined in this document, and read
like it was written by the same hand as the rest of the project.
When in doubt, match the surrounding source code. 

> [!IMPORTANT]
> The keywords *MUST*, *MUST NOT*, *REQUIRED*, *SHALL*, *SHALL NOT*, *SHOULD*,
> *SHOULD NOT*, *RECOMMENDED*, *NOT RECOMMENDED*, *MAY*, and *OPTIONAL* in this
> document are to be interpreted as described in BCP 14 [RFC2119](https://datatracker.ietf.org/doc/rfc2119/)


## Table of Contents
- [0 - Targeting](#0---targeting)
- [1 - Formatting & Style](#1---formatting--style)
  - [(a) - Indentation, Whitespace & Line Length](#1a---indentation-whitespace--line-length)
  - [(b) - Comments](#1b---comments)
  - [(c) - Braces](#1c---braces)
  - [(d) - Conditionals](#1d---conditionals)
  - [(e) - Operators](#1e---operators)
  - [(f) - Naming](#1f---naming)
- [2 - File Structure](#2---file-structure)
  - [(a) - Header Files](#2a---header-files)
  - [(b) - Source Files](#2b---source-files)
  - [(c) - Include Ordering](#2c---include-ordering)
- [3 - Documentation](#3---documentation)
- [4 - Functions](#4---functions)
- [5 - Usage of `goto`](#5---usage-of-goto)
- [6 - Pre-Processors & Macros](#6---pre-processors--macros)
  - [(a) - `./src/config.h` & Build Injected Macros](#6a---srcconfigh--build-injected-macros)
  - [(b) - Conditional Compilation](#6b---conditional-compilation)
  - [(c) - Exiting The Program](#6c---exiting-the-program)
  - [(d) - X-Macro Tables](#6d---x-macro-tables)
- [7 - Standard Practices](#7---standard-practices)
  - [(a) - Error Handling](#7a---error-handling)
  - [(b) - Memory](#7b---memory)
  - [(c) - Constructor / Destructor Pattern](#7c---constructor--destructor-pattern)


---

## 0 - Targeting
The language target is **C99**.

All code must compile using the `-Wall` and `-Wextra` flags, and release code
with the additional `-Werror` flag.
Code must compile under MSVC, GCC, and Clang.

> [!NOTE]
> The above-mentioned flags may be interchangeable for their compiler specific
> variants as needed.


---

## 1 - Formatting & Style
IDE formatting is enforced by [`./.clang-format`](../../.clang-format), with the
following key rules being observed.

> [!IMPORTANT]
> In the event of a conflict between the `./.clang-format` and this document,
> this document shall be used.


---

### 1(a) - Indentation, Whitespace & Line Length
Indentation shall use **tabs** for indentation to allow developers to adjust the
visual width within their IDE to their liking.

Within the core code located in the `./src` directory there is a hard column
limit of **80**, with few exceptions. The absolute column limit is 120.
- *Note*: Code within the `./test` directory is not held to the same hard limit,
  but should make efforts where reasonable to stay within the 80 column limit.

Within `switch` statements, `case` labels are indented 1 level within the switch
body.

Up to two (2) consecutive blank lines are permitted and are used liberally to
separate logical steps inside a function body (see §7).

Files end with a trailing newline.


---

### 1(b) - Comments
Comments starting with `//` are not allowed. Always use `/* comment */`, even
for single-line comments.
```c
// This is comment (wrong)
/* This is comment (ok) */
```

For multi-line comments use `space+asterisk` for every line
```c
/*
 * This is multi-line comments,
 * written in 2 lines (ok)
 */

/**
 * Wrong, use double-asterisk only for doxygen documentation
 */

/*
* Single line comment without space before asterisk (wrong)
*/

/*
 * Single line comment in multi-line configuration (wrong)
 */

/* Single line comment (ok) */
```

Inline comments with code should be avoided.


---

### 1(c) - Braces
Braces use **Allman Style** (i.e. the opening brace goes on its own line for
functions, `struct`/`enum`/`union`, and control statements).

All `case` bodies, `if`/`else` bodies, and `while`/`do`/`for` bodies, trivial
or not, are wrapped in their own brace block.
```c
void token_destroy(token_t* token)
{
	if (NULL == token)
	{
		return;
	}
}

if (NULL == data)
{
	return 0;
}

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


---

### 1(d) - Conditionals
Comparisons against constant values or macros shall use **Yoda Style** (i.e. the
constant is located on the left-hand side of the comparison). This style
provides better readability for constant / macro comparisons, and helps prevent
accidental assignments.
```c
/* BAD */
if (myPointer == NULL)
{
	...
}

/* GOOD */
if (NULL == myPointer)
{
	...
}
```

Long conditionals with multiple conditions should be split onto multiple lines.

Multiline conditionals should where reasonable have one (1) conditional per line
and the opening `(` and closing `)` must be on their own line with no conditionals
on the same line.


---

### 1(e) - Operators
Unary operators should not be separated from their operand.

Binary operators, generally (except for `.` and `->`) should be separated from
their operands by whitespace.

The binary comma operator shall be avoided, which case-by-case exceptions made
for `for` statements.

Outside of macros, the ternary operator shall be avoided, with exceptions made
on a case-by-case basis.

The `*` operator used for defining pointer types shall remain with the data type.
```c
char *get_input(); /* BAD */
char* get_input(); /* GOOD */
```


---

### 1(f) - Naming

| Kind                            | Convention                          | Example                                         |
|---------------------------------|-------------------------------------|-------------------------------------------------|
| Functions                       | `lower_snake_case`, module-prefixed | `buf_reader_read`, `str_copy_safe`              |
| Local variables & parameters    | `lowerCamelCase`                    | `contentLength`, `destSize`, `oldCapacity`      |
| Struct / enum types (typedef'd) | `lower_snake_case` with `_t` suffix | `token_t`, `cli_config_t`, `arr_token_t`        |
| Underlying `struct`/`enum` tag  | `tula_`-prefixed                    | `struct tula_token`, `enum tula_token_type`     |
| Enum members                    | `UPPER_SNAKE_CASE`, group-prefixed  | `TOK_IDENT`, `OPTION_HELP`, `TEST_MODE_SCANNER` |
| Macros & compile-time constants | `UPPER_SNAKE_CASE`                  | `TULA_MAX_BUFFER_SIZE`, `UNREACHABLE_HINT`      |
| Global lookup tables            | `UPPER_SNAKE_CASE`                  | `TOKENS_IS_KEYWORD`, `TEST_MODE_VALUE`          |

Note the deliberate split: **types and functions are snake_case, but variables
are camelCase.** Keep it consistent.

Module functions are prefixed with a short module name that matches the file:
`token_*`, `arr_token_*`, `buf_reader_*`, `str_*`, `cli_*`, `scanner_*`,
`tula_exit_*`.


---

## 2 - File Structure


---

### 2(a) - Header Files
Header files must use **include guards**, not `#pragma once`.

The include guard macro name mirrors the path from the `./src` directory.
- ex. `src/common/strings.h` → `TULA_COMMON_STRINGS_H`,
  `src/engine/scanner/token.h` → `TULA_ENGINE_SCANNER_TOKEN_H`.

The closing `#endif` carries the guard name as a trailing comment.

```c
#ifndef TULA_COMMON_STRINGS_H
#define TULA_COMMON_STRINGS_H
/* ... */
#endif /* TULA_COMMON_STRINGS_H */
```

Public API declarations live in the header with **Doxygen-style doc comments**
(see §6).

Prototypes are separated by a blank line each.

Keep headers minimal: include only what the header itself needs (`<stdbool.h>`,
`<stdint.h>`, etc.).


---

### 2(b) - Source Files
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
**Function Definitions**. Section headers within `./src/config.h` / 
`./src/util.h` use a wider `====` banner style for top-level configuration
blocks - follow whichever style already exists in the file you are editing.


---

### 2(c) - Include Ordering
Includes are grouped, matching `./.clang-format`'s three-priority scheme, with a
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

Project headers are included relative to `./src/` (which is on the include path),
e.g. `#include "common/exit.h"` — not `../../common/exit.h`.


---

## 3 - Documentation
Functions get a Doxygen block in the header using `\brief`, `\param[in]`/
`\param[out]`, `\return`, and `\note`.
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

Inside function bodies, use `/* ... */` block comments to narrate each
step as needed ("Null check our arguments", "Copy the characters up to destSize
- 1"). This step-by-step commenting is a strong house style.


---

## 4 - Functions
Place guard clauses first, and validate arguments (especially `NULL`) as early
as reasonably possible.

Separate each logical stage with a blank line, and logical groups by two (2).
Narrating comments are often useful to also help further clarify logical groups.

Use `const` as a default standard, only deviating as needed.

Declare loop variables in the `for` statement (`for (size_t i = 0; ...)`).

When defining a prototype with no parameters, use the `void` keyword.
```c
uint32_t count_sheep(); /* BAD */
uint32_t count_sheep(void); /* GOOD */
```


---

## 5 - Usage of `goto`
To the dismay of many, `goto` statements are fine to be used where reasonable,
and are not discouraged.

The `goto` statement comes in handy when a function exits from multiple
locations and some common work such as cleanup has to be done. If there is no
cleanup needed then just return directly.


---

## 6 - Pre-Processors & Macros
When using a `#ifdef`/`#ifndef` like macro, the closing `#endif` shall have an
inline comment with the checked condition.


---

### 6(a) - `./src/config.h` & Build Injected Macros
The `./src/config.h` header is used to define some program wide config fields,
as well as to consume macros injected by the build engine.


---

### 6(b) - Conditional Compilation
The same sources build multiple different executables, and code for each is
controlled using macros from the `./src/config.h` header. Specifically, the
`TULA_EXE_*` macros, of which, only 1 is defined so they can be used in a
standard `#ifdef` like statement. Additionally, there are the `TULA_EXE_*_VALUE`
macros which will always evaluate to a `0` or `1` depending on the executable
being built.

There is additional macros to use for conditional compilation, namely the
`COMPILER_*` macros used to determine the compiler being used.

Any conditional compilation which is OS specific should remain in the
`./src/common/os.h` and `./src/common/os.c` files.


---

### 6(c) - Exiting The Program
Exiting the program has been standardized using the tools defined in the
`./src/common/exit.h` and `./src/common/exit.c` files. Anytime the program needs
to exit, regardless of if it's a crash or a successful exit, the macros /
functions defined should be used to maintain consistency.


---

### 6(d) - X-Macro Tables
Enumerations that need parallel data (string names, boolean flags) are defined
once as an X-macro list and expanded multiple ways.
See `DEFINE_TOKENS` in `./src/engine/scanner/token.h` for an example.

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


---

## 7 - Standard Practices


---

### 7(a) - Error Handling
Never call `exit()` or `return` from `main` directly, instead terminate through
the `./src/common/exit.h` API so exit codes and cleanup stay consistent:
- `tula_exit(TULA_EXIT_GOOD)` for normal termination.
- The `tula_exit_err_*` helpers and `exit_err_*_f` macros for error paths,
  which map to the `TULA_EXIT_*` codes.
- Fatal-exit functions are annotated `NO_RETURN`.

Library code may exit fatally on unrecoverable errors (i.e. out-of-memory,
illegal internal state, etc.) by calling `tula_exit_err_no_mem()` /
`tula_exit_err_internal(...)` directly, rather than propagating the error up.
Sometimes the IDE or compiler can't see through the `NO_RETURN`, so follow such
a call with `UNREACHABLE_RETURN(<value>)` to silence "not all paths return"
warnings.


---

### 7(b) - Memory
Check every `malloc` result. On failure inside a `_new` function, free any
partial allocations and return `NULL`:
```c
token->content = malloc(sizeof(char) * (contentLength + 1));
if (NULL == token->content)
{
	free(token);
	return NULL;
}
```

Use the array helpers from `./src/util.h` for growable buffers rather than
open-coding `realloc`: `tula_array_grow_capacity`, `tula_array_resize`,
`tula_array_free` (all routed through `tula_array_reallocate`).


---

### 7(c) - Constructor / Destructor Pattern
Heap-managed types follow a uniform lifecycle naming:
- `xxx_new(...)` — allocates and returns a pointer (or `NULL` on allocation
  failure).
- `xxx_destroy(xxx* )` — frees the object and its owned members; a `NULL`
  argument is a safe no-op.
- `xxx_init(xxx* )` — zero-initializes a caller-owned struct in place.
