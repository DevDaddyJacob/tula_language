#ifndef TULA_LANGUAGE_TULA_H
#define TULA_LANGUAGE_TULA_H

#include <stdint.h>

/*
 * ============================================================================
 * Program definitions
 * ============================================================================
 */

#define TULA_PROGRAM_NAME "tula"

#define TULA_VERSION_MAJOR		"1"
#define TULA_VERSION_MINOR		"0"
#define TULA_VERSION_RELEASE	"0"

#define TULA_VERSION "Tula v" TULA_VERSION_MAJOR "." TULA_VERSION_MINOR
#define TULA_RELEASE TULA_VERSION "." TULA_VERSION_RELEASE

/* ========================================================================= */


/*
 * ============================================================================
 * Common / Utility Macros
 * ============================================================================
 */

#define UNREACHABLE_HINT __builtin_unreachable()

#define UNREACHABLE_RETURN(value) \
	do { UNREACHABLE_HINT; return value; } while(0)
#define UNREACHABLE_DEFAULT(value)  default: UNREACHABLE_HINT; value
#define UNREACHABLE(value)          UNREACHABLE_HINT

#define UNUSED(value) (void)(value)

#define NO_RETURN __attribute__((noreturn))

/* ========================================================================= */


/*
 * ============================================================================
 * Program possible exit codes
 * ============================================================================
 */

#define TULA_EXIT_GOOD						0
#define TULA_EXIT_UNDEF_INTERNAL_ERR		1
#define TULA_EXIT_BAD_USAGE					2
#define TULA_EXIT_NO_MEM					3
#define TULA_EXIT_ACCESS_STATE_BEFORE_INIT	4

#define exit_err_internal_f(format, ...) \
	(tula_exit_error( \
		TULA_EXIT_UNDEF_INTERNAL_ERR, \
		"Undefined Internal Error: " format, \
		__VA_ARGS__ \
		), __builtin_unreachable())

#define exit_err_bad_usage_f(format, ...) \
	(tula_exit_error(TULA_EXIT_BAD_USAGE, "Bad Usage: " format, __VA_ARGS__))

/* ========================================================================= */


NO_RETURN void tula_exit(int32_t exitCode);


NO_RETURN void tula_exit_error(int32_t exitCode, const char* format, ...);


static NO_RETURN void exit_err_internal(const char* msg)
{
	tula_exit_error(
		TULA_EXIT_UNDEF_INTERNAL_ERR,
		"Undefined Internal Error: %s",
		msg
	);
}


static NO_RETURN void exit_err_bad_usage(const char* msg)
{
	tula_exit_error(
		TULA_EXIT_NO_MEM,
		"Bad Usage: %s",
		msg
	);
}


static NO_RETURN void exit_err_no_mem(void)
{
	tula_exit_error(
		TULA_EXIT_NO_MEM,
		"Memory Allocation Error: Failed to allocate memory"
	);
}


static NO_RETURN void exit_err_early_state_access(void)
{
	tula_exit_error(
		TULA_EXIT_ACCESS_STATE_BEFORE_INIT,
		"Early State Access: Attempted to access a state instance before " \
			"it's initialization"
	);
}

#endif /* TULA_LANGUAGE_TULA_H */
