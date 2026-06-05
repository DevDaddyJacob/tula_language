#ifndef TULA_STATE_GSTATE_H
#define TULA_STATE_GSTATE_H

#include <stdint.h>

#include "cli.h"


typedef struct tula_global_state {
	int32_t argc;
	const char** argv;
	cli_config_t* cli;
} global_state_t;


void setup_global_state(int32_t argc, const char** argv);


void teardown_global_state();


global_state_t* get_global_state();

#endif /* TULA_STATE_GSTATE_H */
