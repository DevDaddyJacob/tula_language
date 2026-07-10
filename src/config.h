#ifndef TULA_CONFIG_H
#define TULA_CONFIG_H

/*
 * ============================================================================
 * Executable type config
 * ============================================================================
 */

/*
 * You can use "-DTULA_EXECUTABLE_TYPE=#" as a compiler flag to define this.
 * Supported values include:
 *		- 0: debug exe (tulad)
 *		- 1: full exe (tula)
 *		- 2: compiler exe (tulac)
 *		- 3: runner exe (tular)
 */
#ifndef TULA_EXECUTABLE_TYPE
#	define TULA_EXECUTABLE_TYPE 0
#endif /* TULA_EXECUTABLE_TYPE */


#define TULA_EXE_ALL_VALUE			1
#define TULA_EXE_DEBUG_VALUE		0
#define TULA_EXE_FULL_VALUE			0
#define TULA_EXE_COMPILER_VALUE		0
#define TULA_EXE_RUNNER_VALUE		0

/* Debug (tulad) executable */
#if 0 == TULA_EXECUTABLE_TYPE
#	define TULA_EXE_DEBUG

#   undef TULA_EXE_DEBUG_VALUE
#	define TULA_EXE_DEBUG_VALUE 1

#	define TULA_PROGRAM_NAME "tulad"
#endif

/* Full (tula) executable */
#if 1 == TULA_EXECUTABLE_TYPE
#	define TULA_EXE_FULL

#   undef TULA_EXE_FULL_VALUE
#	define TULA_EXE_FULL_VALUE 1

#	define TULA_PROGRAM_NAME "tula"
#endif

/* Compiler (tulac) executable */
#if 2 == TULA_EXECUTABLE_TYPE
#	define TULA_EXE_COMPILER

#   undef TULA_EXE_COMPILER_VALUE
#	define TULA_EXE_COMPILER_VALUE 1

#	define TULA_PROGRAM_NAME "tulac"
#endif

/* Runner (tular) executable */
#if 3 == TULA_EXECUTABLE_TYPE
#	define TULA_EXE_RUNNER

#   undef TULA_EXE_RUNNER_VALUE
#	define TULA_EXE_RUNNER_VALUE 1

#	define TULA_PROGRAM_NAME "tular"
#endif


#ifndef TULA_PROGRAM_NAME
#	error "Unknown executable type"
#endif

/* ========================================================================= */


/*
 * ============================================================================
 * Configure the operating system macros as well as platform dependent macros
 * ============================================================================
 */

#ifndef IS_OS_DEFINED
#	include "common/os.h"
#endif /* IS_OS_DEFINED */

#ifndef C_COMPILER
#	undef COMPILER_MSVC
#	undef COMPILER_GCC
#	undef COMPILER_CLANG
#	undef COMPILER_CLANG_GCC
#	undef COMPILER_CLANG_MSVC

#	if defined(__clang__)
#		define COMPILER_CLANG
#		if defined(_MSC_VER)
#			define C_COMPILER 1
#			define COMPILER_CLANG_GCC

#		elif defined(__GNUC__)
#			define C_COMPILER 2
#			define COMPILER_CLANG_MSVC

#		else
#			error "Unknown clang compatibility mode"
#		endif

#	elif defined(_MSC_VER)
#		define C_COMPILER 3
#		define COMPILER_MSVC

#	elif defined(__GNUC__)
#		define C_COMPILER 4
#		define COMPILER_GCC

#	else
#		error "Unknown or unsupported compiler"
#	endif
#endif /* C_COMPILER */

/* ========================================================================= */


/*
 * ============================================================================
 * Configure the parameters for arrays
 * ============================================================================
 */

/**
 * The number by which array sizes is multiplied by when increasing their size
 * For example, if an array has 16 elements, and we are wanting to increase the
 * size, we will multiply 16 by the value of TULA_ARRAY_GROW_FACTOR
 */
#define TULA_ARRAY_GROW_FACTOR 2


/**
 * The minimum array size. If we have a minimum of 8 and an array of size 3,
 * and we attempt to resize it, a check will occur to see if the size is lower
 * than the minimum and if so, rather than multiplying by the growth factor,
 * instead we set the size to the minimum.
 */
#define TULA_ARRAY_MIN_THRESHOLD 8

/* ========================================================================= */


/*
 * ============================================================================
 * Configure the parameters for buffers
 * ============================================================================
 */

/**
 * The maximum size a general use buffer can be
 */
#define TULA_MAX_BUFFER_SIZE 2048


/**
 * The size in bytes of any reader's buffer
 */
#define TULA_READER_BUFFER_SIZE 4096

/* ========================================================================= */


/*
 * ============================================================================
 * Configure the parameters for stack traces
 * ============================================================================
 */

/**
 * Maximum call-stack depth captured.
 */
#define TULA_TRACE_MAX_FRAMES 64


/**
 * Maximum length of a resolved symbol name (including null terminator).
 */
#define TULA_TRACE_MAX_SYMB_LEN 256

/* ========================================================================= */


/*
 * ============================================================================
 * Version definitions
 * ============================================================================
*/

#define TULA_LANGUAGE_VERSION \
	TULA_VERSION_MAJOR "." TULA_VERSION_MINOR

#define TULA_RELEASE_VERSION \
	TULA_VERSION_MAJOR "." TULA_VERSION_MINOR "." TULA_VERSION_PATCH

#define TULA_RELEASE_VERSION_DETAILED \
	TULA_RELEASE_VERSION "+hash." TULA_COMMIT_HASH_SHORT

/* ========================================================================= */

#endif /* TULA_CONFIG_H */
