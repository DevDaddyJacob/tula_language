#include <stdarg.h>
#include <stdlib.h>

#include "tula.h"
#include "util.h"
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


void test_lexer() {
	// global_state_t* state = getGlobalState();
	// printf("%s\n", state->cli->file);
	//
	// buffered_file_t* bFile = bufferedFile_new(state->cli->file);
	// lexer_t* lexer = lex_new(bFile);
	//
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


int main(const int32_t argc, const char* argv[]) {
	setup(argc, argv);

	test_lexer();

	tula_exit(TULA_EXIT_GOOD);
}