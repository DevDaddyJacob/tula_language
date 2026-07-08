/*
 * Regression: the scanner mis-classified an identifier that merely *starts*
 * with a keyword as that keyword.
 *
 * try_infer_identifier_type (src/engine/scanner/scanner.c) only checked that a
 * keyword was a prefix of the scanned identifier; it never verified that the
 * identifier ended where the keyword did. As a result identifiers such as
 * "format" (starts with "for"), "vargs" (starts with "var"), "andy", and so on
 * were tokenized as their keyword instead of TOK_IDENT.
 *
 * These cases all route through try_infer_identifier_type. Keywords returned
 * directly (e.g. "if", "do") are a separate matter and are not covered here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "common/buffered_reader.h"
#include "engine/scanner/scanner.h"
#include "engine/scanner/token.h"

#define TEMP_FILE "rt_0001_scanner_keyword_prefix.tula"


typedef struct
{
	const char* text;
	token_type_t expected;
} classification_case_t;


static scanner_t* scanner_for(const char* source)
{
	FILE* file = fopen(TEMP_FILE, "wb");
	if (NULL == file)
	{
		TEST_FAIL_MESSAGE("Could not create temp file");
	}

	fwrite(source, 1, strlen(source), file);
	fclose(file);

	buf_reader_t* reader = buf_reader_new(NULL);
	TEST_ASSERT_TRUE(buf_reader_open(reader, TEMP_FILE));

	return scanner_new(reader);
}


static void scanner_release(scanner_t* scanner)
{
	/*
	 * Close the reader so the temp file handle is released, then free the
	 * scanner shell. scanner_destroy is deliberately avoided: destroying a
	 * populated token array corrupts the heap today (a separate, tracked
	 * scanner bug). The token array is left for process exit to reclaim, which
	 * mirrors how parser_destroy currently defers scanner teardown.
	 */
	buf_reader_destroy(scanner->reader);
	free(scanner);
}


static void run_cases(const classification_case_t* cases, const size_t count)
{
	char source[1024] = { 0 };
	for (size_t i = 0; i < count; i++)
	{
		strcat(source, cases[i].text);
		strcat(source, "\n");
	}

	scanner_t* scanner = scanner_for(source);

	for (size_t i = 0; i < count; i++)
	{
		const token_t* token = scanner_read_next(scanner);
		TEST_ASSERT_NOT_NULL(token);
		TEST_ASSERT_EQUAL_INT_MESSAGE(
			cases[i].expected,
			token->type,
			cases[i].text
		);
	}

	scanner_release(scanner);
}


void setUp(void) { }

void tearDown(void)
{
	remove(TEMP_FILE);
}


/*
 * Identifiers that merely start with a keyword must scan as TOK_IDENT. This is
 * the case the fix restores; every entry below is mis-classified before it.
 */
void test_keyword_prefixed_identifiers_are_identifiers(void)
{
	static const classification_case_t cases[] = {
		{ "format",     TOK_IDENT },	/* "for"      */
		{ "vargs",      TOK_IDENT },	/* "var"      */
		{ "andy",       TOK_IDENT },	/* "and"      */
		{ "orange",     TOK_IDENT },	/* "or"       */
		{ "notation",   TOK_IDENT },	/* "not"      */
		{ "constants",  TOK_IDENT },	/* "constant" */
		{ "functional", TOK_IDENT },	/* "function" */
		{ "returns",    TOK_IDENT },	/* "return"   */
		{ "settings",   TOK_IDENT },	/* "set"      */
		{ "unsettle",   TOK_IDENT },	/* "unset"    */
		{ "globals",    TOK_IDENT },	/* "global"   */
		{ "breaker",    TOK_IDENT },	/* "break"    */
		{ "whiled",     TOK_IDENT },	/* "while"    */
		{ "elsewhere",  TOK_IDENT },	/* "else"     */
		{ "trueish",    TOK_IDENT },	/* "true"     */
		{ "falsey",     TOK_IDENT },	/* "false"    */
		{ "isSetter",   TOK_IDENT },	/* "isSet"    */
	};

	run_cases(cases, sizeof(cases) / sizeof(cases[0]));
}


/*
 * Guard against over-correction: the exact keywords (and their alternate
 * spellings) must still classify as their keyword after the fix.
 */
void test_exact_keywords_still_classify(void)
{
	static const classification_case_t cases[] = {
		{ "for",      TOK_FOR },
		{ "var",      TOK_VARIABLE },
		{ "variable", TOK_VARIABLE },
		{ "and",      TOK_AND },
		{ "or",       TOK_OR },
		{ "not",      TOK_NOT },
		{ "const",    TOK_CONSTANT },
		{ "constant", TOK_CONSTANT },
		{ "func",     TOK_FUNC },
		{ "function", TOK_FUNC },
		{ "return",   TOK_RETURN },
		{ "set",      TOK_SET },
		{ "unset",    TOK_UNSET },
		{ "global",   TOK_GLOBAL },
		{ "break",    TOK_BREAK },
		{ "while",    TOK_WHILE },
		{ "else",     TOK_ELSE },
		{ "true",     TOK_TRUE },
		{ "false",    TOK_FALSE },
		{ "isSet",    TOK_IS_SET },
		{ "continue", TOK_CONTINUE },
		{ "def",      TOK_DEFINE },
		{ "define",   TOK_DEFINE },
	};

	run_cases(cases, sizeof(cases) / sizeof(cases[0]));
}


int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_keyword_prefixed_identifiers_are_identifiers);
	RUN_TEST(test_exact_keywords_still_classify);

	return UNITY_END();
}
