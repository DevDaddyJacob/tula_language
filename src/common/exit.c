#include "exit.h"

#include <stdlib.h>

#include "trace.h"
#include "state/gstate.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

/* #define XYZ "ABC" */

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/**
 * TBD
 */
static void teardown();


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

/* int magicNumber = 420; */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
*/


static void teardown()
{
	teardown_global_state();
}


NO_RETURN void tula_exit(const int32_t exitCode)
{
	teardown();
	exit(exitCode);
}


NO_RETURN void tula_exit_error(
	const int32_t exitCode,
	const bool dumpTrace,
	const char* format,
	...
)
{
	if (NULL != format)
	{
		/* Get the vargs, and print the message */
		va_list vargs;
		va_start(vargs, format);
		err_print_v(format, vargs);
		va_end(vargs);
	}

	if (dumpTrace)
	{
		trace_dump(stderr, 2);
	}

	teardown();
	exit(exitCode);
}