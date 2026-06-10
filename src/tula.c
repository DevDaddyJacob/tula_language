#include <stdarg.h>
#include <stdlib.h>

#include "tula.h"

#include <stdio.h>

#include "util.h"
#include "common/buffered_reader.h"
#include "engine/scanner/scanner.h"
#include "state/gstate.h"


/*
 * ==================================================
 * Macros
 * ==================================================
 */


/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
*/

/**
 * TBD
 */
static void setup(int32_t argc, const char* argv[]);


/**
 * TBD
 */
static void teardown();


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
*/

static void setup(const int32_t argc, const char* argv[])
{
	setup_global_state(argc, argv);
}


static void teardown()
{
	teardown_global_state();
}


void tula_exit(const int32_t exitCode)
{
	teardown();
	exit(exitCode);
}


void tula_exit_error(const int32_t exitCode, const char* format, ...)
{
	if (NULL != format)
	{
		/* Get the vargs, and print the message */
		va_list vargs;
		va_start(vargs, format);
		err_print_v(format, vargs);
		va_end(vargs);
	}

	teardown();
	exit(exitCode);
}

#ifdef TULA_EXE_DEBUGGING
void test_scanner() {
	global_state_t* state = get_global_state();
	printf("%s\n", state->cli->file);

	buf_reader_t* reader = buf_reader_new(NULL);
	buf_reader_open(reader, state->cli->file);
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

	// token_t curToken;
	// int16_t line = -1;
	//
	// do {
	// 	curToken = lex_next(lexer);
	// 	if (curToken.line > line) {
	// 		line = curToken.line;
	// 		printf("Line #%d\n", line);
	// 	}
	//
	// 	printf("\t%-12s\t%s\n", TOKEN_TEXT[curToken.type], curToken.start);
	// } while (
	// 	curToken.type != TOK_EOF
	// 	&& curToken.type != TOK_ERROR
	// );
}
#endif /* TULA_EXE_DEBUGGING */


int main(const int32_t argc, const char* argv[]) {
	setup(argc, argv);

#ifdef TULA_EXE_DEBUGGING
	test_scanner();
#endif /* TULA_EXE_DEBUGGING */

	tula_exit(TULA_EXIT_GOOD);
}