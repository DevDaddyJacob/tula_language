#include "util.h"
#include "common/buffered_reader.h"
#include "common/exit.h"
#include "common/trace.h"
#include "state/gstate.h"


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

/**
 * TBD
 */
static void setup(int32_t argc, const char* argv[]);


/**
 * TBD
 */
static void teardown(void);


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

static void setup(const int32_t argc, const char* argv[])
{
	setup_global_state(argc, argv);
}


static void teardown(void)
{
	teardown_global_state();
}


int main(const int32_t argc, const char* argv[]) {
	setup(argc, argv);

#ifdef TULA_EXE_DEBUG
	run_test();
#endif /* TULA_EXE_DEBUG */

#ifdef TULA_EXE_STANDARD
	// TBD
#endif /* TULA_EXE_STANDARD */

	teardown();
	tula_exit(TULA_EXIT_GOOD);
}