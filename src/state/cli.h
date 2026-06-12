#ifndef TULA_STATE_CLI_H
#define TULA_STATE_CLI_H

#include <stdbool.h>

#include "config.h"
#include "test.h"

typedef struct tula_cli_config {
#ifdef TULA_EXE_STANDARD
	/**
	 * If the interactive option is selected
	 */
	bool interactive;

	/**
	 * The file to run
	 */
	char* file;
#endif /* TULA_EXE_STANDARD */

#ifdef TULA_EXE_DEBUGGING
	test_mode_t testMode;

	char* inputFile;
#endif /* TULA_EXE_DEBUGGING */
} cli_config_t;


/**
 * \brief               Takes the command line arguments and parses them into
 *                      a cli_config_t struct
 */
cli_config_t* cli_parse_args(int32_t argc, const char* argv[]);


void cli_destroy(cli_config_t* config);

#endif /* TULA_STATE_CLI_H */
