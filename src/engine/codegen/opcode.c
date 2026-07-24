#include "opcode.h"

/*
 * ==================================================
 * Macros
 * ==================================================
*/

#define OPCODE_MNEMONIC_DEFINER(identifier, mnemonic, _2, _3) \
	[identifier] = mnemonic,

#define OPCODE_SIZE_DEFINER(identifier, _1, _2, size) \
	[identifier] = size,


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/**
 * TBD
 */
/* static void example(); */


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
*/

const char* OPCODES_MNEMONIC[TOTAL_OPCODES] = {
	DEFINE_OPCODES(OPCODE_MNEMONIC_DEFINER)
};

const int32_t OPCODES_SIZE[TOTAL_OPCODES] = {
	DEFINE_OPCODES(OPCODE_SIZE_DEFINER)
};


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

/* static void example(); { return; } */