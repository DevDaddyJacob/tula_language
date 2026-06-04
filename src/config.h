#ifndef TULA_LANGUAGE_CONFIG_H
#define TULA_LANGUAGE_CONFIG_H

/*
 * ============================================================================
 * Development debugging config
 * ============================================================================
 */

/**
 * Used manually if you want to have some extra debugging output, mainly for
 * development work
 */
#define TULA_DEBUGGING

/* ========================================================================= */


/*
 * ============================================================================
 * Configure the operating system macros as well as platform dependent macros
 * ============================================================================
 */

#ifndef IS_OS_DEFINED
#include "common/os.h"
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
 * The maximum size a buffer can be
 */
#define TULA_MAX_BUFFER_SIZE 2048

/* ========================================================================= */


/*
 * ============================================================================
 * Configure the parameters for file scanner
 * ============================================================================
 */

/**
 * The size in bytes of the scanner's buffer
 */
#define TULA_SCANNER_BUFFER_SIZE 4096

/* ========================================================================= */

#endif /* TULA_LANGUAGE_CONFIG_H */
