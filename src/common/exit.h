#ifndef TULA_COMMON_EXIT_H
#define TULA_COMMON_EXIT_H

#include <stdint.h>

#include "util.h"

/* Program possible exit codes */
#define TULA_EXIT_GOOD						0
#define TULA_EXIT_UNDEF_INTERNAL_ERR		1
#define TULA_EXIT_BAD_USAGE					2
#define TULA_EXIT_NO_MEM					3
#define TULA_EXIT_ACCESS_STATE_BEFORE_INIT	4

#define exit_err_internal_f(format, ...) \
	(tula_exit_error( \
		TULA_EXIT_UNDEF_INTERNAL_ERR, \
		false, \
		"Undefined Internal Error: " format, \
		__VA_ARGS__ \
		), __builtin_unreachable())

#define exit_err_bad_usage_f(format, ...) \
	(tula_exit_error( \
		TULA_EXIT_BAD_USAGE, \
		false, \
		"Bad Usage: " format, \
		__VA_ARGS__\
		))


NO_RETURN void tula_exit(int32_t exitCode);


NO_RETURN void tula_exit_error(
	int32_t exitCode,
	bool dumpTrace,
	const char* format,
	...
);


static NO_RETURN void tula_exit_err_internal(const char* msg)
{
	tula_exit_error(
		TULA_EXIT_UNDEF_INTERNAL_ERR,
		false,
		"Undefined Internal Error: %s",
		msg
	);
}


static NO_RETURN void tula_exit_err_bad_usage(const char* msg)
{
	tula_exit_error(
		TULA_EXIT_NO_MEM,
		true,
		"Bad Usage: %s",
		msg
	);
}


static NO_RETURN void tula_exit_err_no_mem(void)
{
	tula_exit_error(
		TULA_EXIT_NO_MEM,
		true,
		"Memory Allocation Error: Failed to allocate memory"
	);
}


static NO_RETURN void tula_exit_err_early_state_access(void)
{
	tula_exit_error(
		TULA_EXIT_ACCESS_STATE_BEFORE_INIT,
		false,
		"Early State Access: Attempted to access a state instance before " \
			"it's initialization"
	);
}

#endif /* TULA_COMMON_EXIT_H */
