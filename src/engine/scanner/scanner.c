#include "scanner.h"

/*
 * ==================================================
 * Macros
 * ==================================================
*/

#define TOKEN_VALUE_DEFINER(identifier, value, _2, _3, _4, _5, _6) \
	[identifier] = value,

#define TOKEN_VALUE_ALT_DEFINER(identifier, _1, valueAlt, _3, _4, _5, _6) \
	[identifier] = valueAlt,

#define TOKEN_IS_META_DEFINER(identifier, _1, _2, isMeta, _4, _5, _6) \
	[identifier] = isMeta,

#define TOKEN_IS_PRIMITIVE_DEFINER(identifier, _1, _2, _3, isPrimitive, _5, _6) \
	[identifier] = isPrimitive,

#define TOKEN_IS_KEYWORD_DEFINER(identifier, _1, _2, _3, _4, isKeyword, _6) \
	[identifier] = isKeyword,

#define TOKEN_IS_OPERATOR_DEFINER(identifier, _1, _2, _3, _4, _5, isOperator) \
	[identifier] = isOperator,


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

const char* TOKENS_VALUE[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_VALUE_DEFINER)
};

const char* TOKENS_VALUE_ALT[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_VALUE_ALT_DEFINER)
};

const uint8_t TOKENS_IS_META[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_META_DEFINER)
};

const uint8_t TOKENS_IS_PRIMITIVE[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_PRIMITIVE_DEFINER)
};

const uint8_t TOKENS_IS_KEYWORD[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_KEYWORD_DEFINER)
};

const uint8_t TOKENS_IS_OPERATOR[TOTAL_TOKENS] = {
	DEFINE_TOKENS(TOKEN_IS_OPERATOR_DEFINER)
};


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

/* static void example(); { return; } */