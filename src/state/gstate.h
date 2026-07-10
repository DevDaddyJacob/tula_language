#ifndef TULA_STATE_GSTATE_H
#define TULA_STATE_GSTATE_H

#include <stdint.h>

#include "cli.h"


struct tula_global_state {
	int32_t argc;
	const char** argv;
	struct tula_cli_config* cli;
};


void setup_global_state(int32_t argc, const char** argv);


void teardown_global_state(void);


struct tula_global_state* get_global_state(void);

#endif /* TULA_STATE_GSTATE_H */
