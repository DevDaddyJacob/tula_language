#ifndef TULA_LANGUAGE_TULA_H
#define TULA_LANGUAGE_TULA_H

#include "config.h"

/*
 * ============================================================================
 * Program definitions
 * ============================================================================
 */

#if defined(TULA_EXE_STANDARD)
#	define TULA_PROGRAM_NAME "tula"
#elif defined(TULA_EXE_DEBUGGING)
#	define TULA_PROGRAM_NAME "tulad"
#else
#	error "Unknown executable type"
#endif

#define TULA_VERSION_MAJOR		"1"
#define TULA_VERSION_MINOR		"0"
#define TULA_VERSION_RELEASE	"0"

#define TULA_VERSION "Tula v" TULA_VERSION_MAJOR "." TULA_VERSION_MINOR
#define TULA_RELEASE TULA_VERSION "." TULA_VERSION_RELEASE

/* ========================================================================= */

#endif /* TULA_LANGUAGE_TULA_H */
