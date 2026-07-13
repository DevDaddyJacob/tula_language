#include <stdint.h>
#include <stdlib.h>

#include "gstate.h"
#include "cli.h"
#include "common/exit.h"


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

static struct tula_global_state* GLOBAL_STATE = NULL;


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
	GLOBAL_STATE = (struct tula_global_state*)malloc(
		sizeof(struct tula_global_state)
	);

	if (NULL == GLOBAL_STATE)
	{
		tula_exit_err_no_mem();
		UNREACHABLE_RETURN();
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
	if (NULL == GLOBAL_STATE)
	{
		return;
	}


	/* Destroy the cli config */
	if (NULL != GLOBAL_STATE->cli)
	{
		cli_destroy(GLOBAL_STATE->cli);
		GLOBAL_STATE->cli = NULL;
	}


	free(GLOBAL_STATE);
	GLOBAL_STATE = NULL;
}


struct tula_global_state* get_global_state()
{
	if (NULL == GLOBAL_STATE)
	{
		tula_exit_err_early_state_access();
		UNREACHABLE_RETURN(NULL);
	}

	return GLOBAL_STATE;
}