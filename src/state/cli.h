#ifndef TULAC_STATE_CLI_H
#define TULAC_STATE_CLI_H

#include "../tula.h"

typedef struct tula_cli_config {
	/**
	 * If the interactive option is selected
	 */
	uint8_t interactive;

	/**
	 * The file to run
	 */
	char* file;
} cli_config_t;


/**
 * \brief               Takes the command line arguments and parses them into
 *                      a cli_config_t struct
 */
cli_config_t* cli_parseArgs(int32_t argc, const char* argv[]);


void cli_destroy(cli_config_t* config);

#endif /* TULAC_STATE_CLI_H */
