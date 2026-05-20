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

#ifndef _TULA_OS_DEF // NOLINT(*-reserved-identifier)
    #undef TULA_OS_WINDOWS
    #undef TULA_OS_MAC
    #undef TULA_OS_UNIX
    #undef TULA_OS_POSIX_COMPLIANT

    #if defined(_WIN32) || defined(_WIN64)
        #define TULA_OS_WINDOWS
        #define _TULA_OS_DEF

    #elif defined(__APPLE__) && defined(__MACH__)
        #define TULA_OS_MAC
        #define TULA_OS_POSIX_COMPLIANT
        #define _TULA_OS_DEF

    #elif defined(__linux__) || defined(__unix__)
        #define TULA_OS_UNIX
        #define TULA_OS_POSIX_COMPLIANT
        #define _TULA_OS_DEF

    #else
        #error "Unknown or unsupported platform"
    #endif
#endif /* _TULA_OS_DEF */


#ifndef TULA_OS_PATH_SEPARATOR
    #if defined(TULA_OS_WINDOWS)
        #define TULA_OS_PATH_SEPARATOR '\\'
    #elif defined(TULA_OS_POSIX_COMPLIANT)
        #define TULA_OS_PATH_SEPARATOR '/'
    #else
        #error "Unsupported platform, " \
            "no compliant implementation of macro 'TULA_OS_PATH_SEPARATOR'."
    #endif
#endif /* TULA_OS_PATH_SEPARATOR */


#ifndef TULA_OS_MAX_PATH_LENGTH
    #if defined(TULA_OS_WINDOWS)
        #define TULA_OS_MAX_PATH_LENGTH 260
    #elif defined(TULA_OS_MAC)
        #define TULA_OS_MAX_PATH_LENGTH 1024
    #elif defined(TULA_OS_POSIX_COMPLIANT)
        #define TULA_OS_MAX_PATH_LENGTH 4096
    #else
        #error "Unsupported platform, " \
            "no compliant implementation of macro 'MAX_PATH_LENGTH'."
    #endif
#endif /* TULA_OS_MAX_PATH_LENGTH */

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
