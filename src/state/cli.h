#ifndef TULA_STATE_CLI_H
#define TULA_STATE_CLI_H

#include <stdbool.h>

#include "config.h"
#include "test.h"

struct tula_cli_config {
	int8_t dummy: 1;

#ifdef TULA_EXE_FULL
	/**
	 * If the interactive option is selected
	 */
	bool interactive;

	/**
	 * The file to run
	 */
	char* file;
#endif /* TULA_EXE_FULL */

#ifdef TULA_EXE_DEBUG
	test_mode_t testMode;

	char* inputFile;
#endif /* TULA_EXE_DEBUG */
};


/**
 * \brief               Takes the command line arguments and parses them into
 *                      a tula_cli_config struct
 */
struct tula_cli_config* cli_parse_args(int32_t argc, const char* argv[]);


void cli_destroy(struct tula_cli_config* config);

#endif /* TULA_STATE_CLI_H */
