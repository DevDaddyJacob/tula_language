#ifndef TULA_LANGUAGE_CONFIG_H
#define TULA_LANGUAGE_CONFIG_H

/*
 * ============================================================================
 * Executable type config
 * ============================================================================
 */

/*
 * You can use "-DTULA_EXECUTABLE_TYPE=#" as a compiler flag to define this.
 * Supported values include:
 *		- 0: standard mode
 *		- 1: debugging mode
 */
#ifndef TULA_EXECUTABLE_TYPE
#	define TULA_EXECUTABLE_TYPE 1
#endif /* TULA_EXECUTABLE_TYPE */


/* Standard executable mode */
#if 0 == TULA_EXECUTABLE_TYPE
#	define TULA_EXE_STANDARD
#endif


/* Debugging executable mode */
#if 1 == TULA_EXECUTABLE_TYPE
#	define TULA_EXE_DEBUGGING
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

#endif /* TULA_LANGUAGE_CONFIG_H */
