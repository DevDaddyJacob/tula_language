#include "test.h"

#include <stdio.h>

#include "tula.h"
#include "common/buffered_reader.h"
#include "common/exit.h"
#include "engine/parser/parser.h"
#include "engine/scanner/scanner.h"
#include "state/gstate.h"

#ifdef TULA_EXE_DEBUGGING

/*
 * ==================================================
 * Macros
 * ==================================================
 */

#define TEST_MODE_VALUE_DEFINER(identifier, value) \
	[identifier] = value,

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

static int32_t test_version();

static int32_t test_scanner(const char* inputFilePath);

static int32_t test_parser(const char* inputFilePath);


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
*/

const char* TEST_MODE_VALUE[TOTAL_TEST_MODES] = {
	DEFINE_TEST_MODES(TEST_MODE_VALUE_DEFINER)
};


/*
 * ==================================================
 * Function Definitions
 * ==================================================
*/

static int32_t test_version()
{
	printf(TULA_RELEASE "\n");

	return TULA_EXIT_GOOD;
}


static int32_t test_scanner(const char* inputFilePath)
{
	buf_reader_t* reader = buf_reader_new(NULL);
	if (!buf_reader_open(reader, inputFilePath))
	{
		return TULA_EXIT_UNDEF_INTERNAL_ERR;
	}

	scanner_t* scanner = scanner_new(reader);

	const token_t* lastToken = scanner_read_next(scanner);
	do {
		token_print(lastToken);
		printf("\n");

		lastToken = scanner_read_next(scanner);
	}
	while (
		NULL != lastToken
		&& TOK_ERROR != lastToken->type
		&& TOK_EOS != lastToken->type
	);

	token_print(lastToken);
	printf("\n");

	return TULA_EXIT_GOOD;
}


static int32_t test_parser(const char* inputFilePath)
{
	buf_reader_t* reader = buf_reader_new(NULL);
	if (!buf_reader_open(reader, inputFilePath))
	{
		return TULA_EXIT_UNDEF_INTERNAL_ERR;
	}

	/* The parser owns the scanner (and its reader) and the AST it produces */
	scanner_t* scanner = scanner_new(reader);
	parser_t* parser = parser_new(scanner);

	const ast_node_t* ast = parser_parse(parser);
	ast_node_print(ast);

	parser_destroy(parser);

	return TULA_EXIT_GOOD;
}

int32_t run_test(void)
{
	const global_state_t* state = get_global_state();

	switch (state->cli->testMode)
	{
		case TEST_MODE_VERSION:
		{
			return test_version();
		}

		case TEST_MODE_SCANNER:
		{
			return test_scanner(state->cli->inputFile);
		}

		case TEST_MODE_PARSER:
		{
			return test_parser(state->cli->inputFile);
		}

		default:
		{
			return TULA_EXIT_UNDEF_INTERNAL_ERR;
		}
	}
}


#endif /* TULA_EXE_DEBUGGING */
