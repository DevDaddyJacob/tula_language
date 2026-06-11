#include <stdint.h>
#include <stdlib.h>

#include "gstate.h"
#include "cli.h"
#include "../tula.h"
#include "common/exit.h"

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


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

static global_state_t* GLOBAL_STATE = NULL;


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

void setup_global_state(const int32_t argc, const char** argv)
{
	if (NULL != GLOBAL_STATE)
	{
		return;
	}

	/* Initialize the state in heap */
	GLOBAL_STATE = (global_state_t*)malloc(sizeof(global_state_t));
	if (NULL == GLOBAL_STATE)
	{
		tula_exit_err_no_mem();
	}


	/* Initialize all fields */
	GLOBAL_STATE->argc = argc;
	GLOBAL_STATE->argv = argv;
	GLOBAL_STATE->cli = NULL;


	/* Set up the cli config */
	GLOBAL_STATE->cli = cli_parse_args(
		GLOBAL_STATE->argc,
		GLOBAL_STATE->argv
	);
}


void teardown_global_state()
{
	if (GLOBAL_STATE == NULL)
	{
		return;
	}


	/* Destroy the cli config */
	if (GLOBAL_STATE->cli != NULL)
	{
		cli_destroy(GLOBAL_STATE->cli);
		GLOBAL_STATE->cli = NULL;
	}


	free(GLOBAL_STATE);
	GLOBAL_STATE = NULL;
}


global_state_t* get_global_state()
{
	if (NULL == GLOBAL_STATE)
	{
		tula_exit_err_early_state_access();
	}

	return GLOBAL_STATE;
}