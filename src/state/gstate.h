#ifndef TULA_STATE_GSTATE_H
#define TULA_STATE_GSTATE_H

#include <stdint.h>

#include "cli.h"


typedef struct tula_global_state {
	int32_t argc;
	const char** argv;
	cli_config_t* cli;
} global_state_t;


void setupGlobalState(int32_t argc, const char** argv);


void teardownGlobalState();


global_state_t* getGlobalState();

#endif /* TULA_STATE_GSTATE_H */
