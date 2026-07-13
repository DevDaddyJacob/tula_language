/*
 * Regression: arr_token_destroy corrupted the heap when destroying a token
 * array that held more than one token.
 *
 * arr_token_add (src/engine/scanner/token.c) copies each token *by value* into
 * the contiguous `values` buffer. arr_token_destroy, however, called
 * token_destroy(array->values + i) on every element. token_destroy ends with
 * free(token), so this handed free() an interior pointer into the single
 * `values` allocation for every i > 0 -- undefined behaviour that corrupts the
 * heap (an access violation on MSVC) once the array holds two or more tokens.
 * The `values` buffer itself was also never released, leaking on every destroy.
 *
 * The fix frees each element's owned `content` string, then frees the `values`
 * buffer once, then the array shell -- never an interior pointer.
 *
 * This test builds multi-token arrays (the failing case) and drives them
 * through arr_token_destroy. Before the fix the destroy corrupts the heap /
 * crashes the process, so the test fails; after the fix it returns cleanly.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "common/buffered_reader.h"
#include "engine/scanner/scanner.h"
#include "engine/scanner/token.h"

#define TEMP_FILE "rt_0002_token_array_destroy.tula"


void setUp(void) { }

void tearDown(void)
{
	remove(TEMP_FILE);
}


/*
 * Build an array by hand with several tokens, mirroring how the scanner feeds
 * arr_token_add: token_new allocates the shell, arr_token_add copies it in by
 * value (so the array owns the copied `content`), and the original shell is
 * left for the caller (the scanner never frees it either).
 *
 * Destroying the populated array must not corrupt the heap. Before the fix this
 * crashes on the second element's interior-pointer free.
 */
static void destroy_manual_array(const size_t count)
{
	arr_token_t* array = malloc(sizeof(arr_token_t));
	TEST_ASSERT_NOT_NULL(array);
	arr_token_init(array);

	for (size_t i = 0; i < count; i++)
	{
		/* Mix content-bearing tokens and NULL-content tokens (e.g. EOS). */
		token_t* token = (0 == (i % 3))
			? token_new_eos(1, (uint32_t)i)
			: token_new(TOK_IDENT, 1, (uint32_t)i, "ident", 5);

		TEST_ASSERT_NOT_NULL(token);
		arr_token_add(array, token);

		/*
		 * Free only the shell; `content` ownership moved into the array copy.
		 * (The scanner leaks this shell instead -- either way arr_token_destroy
		 * must own and release the copied content exactly once.)
		 */
		free(token);
	}

	TEST_ASSERT_EQUAL_UINT64(count, array->count);

	arr_token_destroy(array);
}


void test_destroy_multi_token_array_is_safe(void)
{
	destroy_manual_array(5);
	TEST_PASS_MESSAGE("Destroyed a 5-token array without heap corruption");
}


void test_destroy_edge_cases(void)
{
	/* Empty array: nothing allocated in `values`. */
	arr_token_t* empty = malloc(sizeof(arr_token_t));
	TEST_ASSERT_NOT_NULL(empty);
	arr_token_init(empty);
	arr_token_destroy(empty);

	/* Single token: the boundary the buggy code happened to survive. */
	destroy_manual_array(1);

	/* NULL is tolerated. */
	arr_token_destroy(NULL);

	TEST_PASS();
}


/*
 * The real-world path: a scanner that has consumed a whole token stream must be
 * safe to tear down. scanner_destroy calls arr_token_destroy, so before the fix
 * this crashes; after it, teardown is clean.
 */
void test_scanner_destroy_after_scan_is_safe(void)
{
	static const char* source = "def add func(a b) return a + b\n";

	FILE* file = fopen(TEMP_FILE, "wb");
	if (NULL == file)
	{
		TEST_FAIL_MESSAGE("Could not create temp file");
	}
	fwrite(source, 1, strlen(source), file);
	fclose(file);

	buf_reader_t* reader = buf_reader_new(NULL);
	TEST_ASSERT_TRUE(buf_reader_open(reader, TEMP_FILE));

	scanner_t* scanner = scanner_new(reader);
	TEST_ASSERT_NOT_NULL(scanner);

	for (int i = 0; i < 16; i++)
	{
		const token_t* token = scanner_read_next(scanner);
		TEST_ASSERT_NOT_NULL(token);
		if (TOK_EOS == token->type)
		{
			break;
		}
	}

	scanner_destroy(scanner);

	TEST_PASS_MESSAGE("Tore down a populated scanner without heap corruption");
}


int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_destroy_multi_token_array_is_safe);
	RUN_TEST(test_destroy_edge_cases);
	RUN_TEST(test_scanner_destroy_after_scan_is_safe);

	return UNITY_END();
}
