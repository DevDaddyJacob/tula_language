#include "unity.h"
#include "unity_memory.h"
#include "common/buffered_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ==================================================
 * Helpers
 * ==================================================
 */

#define TEMP_FILE "test_buf_reader_tmp.txt"

static void write_temp_file(const char* content)
{
    FILE* f = fopen(TEMP_FILE, "wb");
    if (NULL == f)
    {
    	TEST_FAIL_MESSAGE("Could not create temp file");
    }

    fwrite(content, 1, strlen(content), f);
    fclose(f);
}

static void remove_temp_file(void)
{
    remove(TEMP_FILE);
}

/*
 * ==================================================
 * setUp / tearDown
 * ==================================================
 */

void setUp(void)
{
    UnityMalloc_StartTest();
}

void tearDown(void)
{
    remove_temp_file();
    UnityMalloc_EndTest();
}


/* -------------------------------------------------------------------------
 * buf_reader_new
 * ------------------------------------------------------------------------- */

void test_buf_reader_new_with_filename_succeeds(void)
{
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_FALSE(buf_reader_is_open(r));
    TEST_ASSERT_FALSE(buf_reader_has_error(r));
    buf_reader_destroy(r);
}

void test_buf_reader_new_with_null_filename_succeeds(void)
{
    /* NULL fileName is allowed; open must be called later with a name */
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_FALSE(buf_reader_is_open(r));
    buf_reader_destroy(r);
}

void test_buf_reader_new_initial_state(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_FALSE(buf_reader_is_open(r));
    TEST_ASSERT_FALSE(buf_reader_has_error(r));
    TEST_ASSERT_TRUE(buf_reader_has_next(r));   /* not EOF yet */
    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_destroy
 * ------------------------------------------------------------------------- */

void test_buf_reader_destroy_null_is_safe(void)
{
    /* Must not crash */
    buf_reader_destroy(NULL);
    TEST_PASS();
}

void test_buf_reader_destroy_closes_open_file(void)
{
    write_temp_file("hello");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    TEST_ASSERT_NOT_NULL(r);
    buf_reader_open(r, NULL);
    TEST_ASSERT_TRUE(buf_reader_is_open(r));
    buf_reader_destroy(r);   /* must not crash / leak */
    TEST_PASS();
}


/* -------------------------------------------------------------------------
 * buf_reader_open
 * ------------------------------------------------------------------------- */

void test_buf_reader_open_existing_file_succeeds(void)
{
    write_temp_file("hello");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_TRUE(buf_reader_open(r, NULL));
    TEST_ASSERT_TRUE(buf_reader_is_open(r));
    TEST_ASSERT_FALSE(buf_reader_has_error(r));
    buf_reader_destroy(r);
}

void test_buf_reader_open_nonexistent_file_fails(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_FALSE(buf_reader_open(r, "no_such_file_xyz.txt"));
    TEST_ASSERT_FALSE(buf_reader_is_open(r));
    TEST_ASSERT_TRUE(buf_reader_has_error(r));
    buf_reader_destroy(r);
}

void test_buf_reader_open_null_reader_returns_false(void)
{
    TEST_ASSERT_FALSE(buf_reader_open(NULL, TEMP_FILE));
}

void test_buf_reader_open_null_filename_and_null_stored_returns_false(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_FALSE(buf_reader_open(r, NULL));
    buf_reader_destroy(r);
}

void test_buf_reader_open_with_explicit_filename_overrides_stored(void)
{
    write_temp_file("data");
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_TRUE(buf_reader_open(r, TEMP_FILE));
    TEST_ASSERT_TRUE(buf_reader_is_open(r));
    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_close
 * ------------------------------------------------------------------------- */

void test_buf_reader_close_null_returns_false(void)
{
    TEST_ASSERT_FALSE(buf_reader_close(NULL));
}

void test_buf_reader_close_resets_state(void)
{
    write_temp_file("hello");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);
    TEST_ASSERT_TRUE(buf_reader_is_open(r));

    TEST_ASSERT_TRUE(buf_reader_close(r));
    TEST_ASSERT_FALSE(buf_reader_is_open(r));
    TEST_ASSERT_FALSE(buf_reader_has_error(r));
    TEST_ASSERT_TRUE(buf_reader_has_next(r));   /* EOF flag reset */
    buf_reader_destroy(r);
}

void test_buf_reader_close_allows_reopen(void)
{
    write_temp_file("hi");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);
    buf_reader_close(r);

    TEST_ASSERT_TRUE(buf_reader_open(r, NULL));
    TEST_ASSERT_TRUE(buf_reader_is_open(r));
    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_read
 * ------------------------------------------------------------------------- */

void test_buf_reader_read_null_returns_minus_one(void)
{
    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_read(NULL));
}

void test_buf_reader_read_returns_chars_in_order(void)
{
    write_temp_file("ABC");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32('A', buf_reader_read(r));
    TEST_ASSERT_EQUAL_INT32('B', buf_reader_read(r));
    TEST_ASSERT_EQUAL_INT32('C', buf_reader_read(r));

    buf_reader_destroy(r);
}

void test_buf_reader_read_past_eof_returns_minus_one(void)
{
    write_temp_file("X");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    buf_reader_read(r);                                    /* consume 'X' */
    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_read(r));       /* EOF */

    buf_reader_destroy(r);
}

void test_buf_reader_read_empty_file_returns_minus_one(void)
{
    write_temp_file("");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_read(r));

    buf_reader_destroy(r);
}

void test_buf_reader_read_advances_column_number(void)
{
    write_temp_file("AB");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    buf_reader_read(r);
    TEST_ASSERT_EQUAL_UINT32(1, r->columnNumber);
    buf_reader_read(r);
    TEST_ASSERT_EQUAL_UINT32(2, r->columnNumber);

    buf_reader_destroy(r);
}

void test_buf_reader_read_newline_increments_line_number(void)
{
    write_temp_file("A\nB");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    buf_reader_read(r);                                  /* 'A' */
    TEST_ASSERT_EQUAL_UINT32(1, r->lineNumber);

    buf_reader_read(r);                                  /* '\n' */
    TEST_ASSERT_EQUAL_UINT32(2, r->lineNumber);
    TEST_ASSERT_EQUAL_UINT32(1, r->columnNumber);

    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_peek
 * ------------------------------------------------------------------------- */

void test_buf_reader_peek_null_returns_minus_one(void)
{
    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_peek(NULL));
}

void test_buf_reader_peek_does_not_advance(void)
{
    write_temp_file("AB");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32('A', buf_reader_peek(r));
    TEST_ASSERT_EQUAL_INT32('A', buf_reader_peek(r));  /* still 'A' */

    buf_reader_destroy(r);
}

void test_buf_reader_peek_then_read_returns_same_char(void)
{
    write_temp_file("Z");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32('Z', buf_reader_peek(r));
    TEST_ASSERT_EQUAL_INT32('Z', buf_reader_read(r));

    buf_reader_destroy(r);
}

void test_buf_reader_peek_at_eof_returns_minus_one(void)
{
    write_temp_file("X");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    buf_reader_read(r);
    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_peek(r));

    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_peek_n
 * ------------------------------------------------------------------------- */

void test_buf_reader_peek_n_null_returns_minus_one(void)
{
    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_peek_n(NULL, 0));
}

void test_buf_reader_peek_n_zero_returns_current_char(void)
{
    write_temp_file("ABC");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32('A', buf_reader_peek_n(r, 0));

    buf_reader_destroy(r);
}

void test_buf_reader_peek_n_lookahead(void)
{
    write_temp_file("ABC");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32('B', buf_reader_peek_n(r, 1));
    TEST_ASSERT_EQUAL_INT32('C', buf_reader_peek_n(r, 2));

    buf_reader_destroy(r);
}

void test_buf_reader_peek_n_does_not_advance(void)
{
    write_temp_file("ABC");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    buf_reader_peek_n(r, 2);
    TEST_ASSERT_EQUAL_INT32('A', buf_reader_read(r));  /* position unchanged */

    buf_reader_destroy(r);
}

void test_buf_reader_peek_n_beyond_content_returns_minus_one(void)
{
    write_temp_file("AB");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_peek_n(r, 5));

    buf_reader_destroy(r);
}

void test_buf_reader_peek_n_exceeds_buffer_returns_minus_one(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_NOT_NULL(r);
    /* n >= BUF_READER_BUFFER_SIZE is rejected outright */
    TEST_ASSERT_EQUAL_INT32(-1, buf_reader_peek_n(r, BUF_READER_BUFFER_SIZE));
    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_consume
 * ------------------------------------------------------------------------- */

void test_buf_reader_consume_null_is_safe(void)
{
    buf_reader_consume(NULL);   /* must not crash */
    TEST_PASS();
}

void test_buf_reader_consume_advances_position(void)
{
    write_temp_file("AB");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    TEST_ASSERT_EQUAL_INT32('A', buf_reader_peek(r));
    buf_reader_consume(r);
    TEST_ASSERT_EQUAL_INT32('B', buf_reader_peek(r));

    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_is_open
 * ------------------------------------------------------------------------- */

void test_buf_reader_is_open_null_returns_false(void)
{
    TEST_ASSERT_FALSE(buf_reader_is_open(NULL));
}

void test_buf_reader_is_open_before_open_returns_false(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_FALSE(buf_reader_is_open(r));
    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_has_next
 * ------------------------------------------------------------------------- */

void test_buf_reader_has_next_null_returns_false(void)
{
    TEST_ASSERT_FALSE(buf_reader_has_next(NULL));
}

void test_buf_reader_has_next_true_before_eof(void)
{
    write_temp_file("A");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);
    TEST_ASSERT_TRUE(buf_reader_has_next(r));
    buf_reader_destroy(r);
}

void test_buf_reader_has_next_false_after_eof(void)
{
    write_temp_file("X");
    buf_reader_t* r = buf_reader_new(TEMP_FILE);
    buf_reader_open(r, NULL);

    buf_reader_read(r);        /* consume 'X'  */
    buf_reader_read(r);        /* trigger EOF  */

    TEST_ASSERT_FALSE(buf_reader_has_next(r));

    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * buf_reader_has_error
 * ------------------------------------------------------------------------- */

void test_buf_reader_has_error_null_returns_false(void)
{
    TEST_ASSERT_FALSE(buf_reader_has_error(NULL));
}

void test_buf_reader_has_error_false_on_clean_reader(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    TEST_ASSERT_FALSE(buf_reader_has_error(r));
    buf_reader_destroy(r);
}

void test_buf_reader_has_error_true_after_missing_file(void)
{
    buf_reader_t* r = buf_reader_new(NULL);
    buf_reader_open(r, "definitely_missing_file.txt");
    TEST_ASSERT_TRUE(buf_reader_has_error(r));
    buf_reader_destroy(r);
}


/* -------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_buf_reader_new_with_filename_succeeds);
    RUN_TEST(test_buf_reader_new_with_null_filename_succeeds);
    RUN_TEST(test_buf_reader_new_initial_state);

    RUN_TEST(test_buf_reader_destroy_null_is_safe);
    RUN_TEST(test_buf_reader_destroy_closes_open_file);

    RUN_TEST(test_buf_reader_open_existing_file_succeeds);
    RUN_TEST(test_buf_reader_open_nonexistent_file_fails);
    RUN_TEST(test_buf_reader_open_null_reader_returns_false);
    RUN_TEST(test_buf_reader_open_null_filename_and_null_stored_returns_false);
    RUN_TEST(test_buf_reader_open_with_explicit_filename_overrides_stored);

    RUN_TEST(test_buf_reader_close_null_returns_false);
    RUN_TEST(test_buf_reader_close_resets_state);
    RUN_TEST(test_buf_reader_close_allows_reopen);

    RUN_TEST(test_buf_reader_read_null_returns_minus_one);
    RUN_TEST(test_buf_reader_read_returns_chars_in_order);
    RUN_TEST(test_buf_reader_read_past_eof_returns_minus_one);
    RUN_TEST(test_buf_reader_read_empty_file_returns_minus_one);
    RUN_TEST(test_buf_reader_read_advances_column_number);
    RUN_TEST(test_buf_reader_read_newline_increments_line_number);

    RUN_TEST(test_buf_reader_peek_null_returns_minus_one);
    RUN_TEST(test_buf_reader_peek_does_not_advance);
    RUN_TEST(test_buf_reader_peek_then_read_returns_same_char);
    RUN_TEST(test_buf_reader_peek_at_eof_returns_minus_one);

    RUN_TEST(test_buf_reader_peek_n_null_returns_minus_one);
    RUN_TEST(test_buf_reader_peek_n_zero_returns_current_char);
    RUN_TEST(test_buf_reader_peek_n_lookahead);
    RUN_TEST(test_buf_reader_peek_n_does_not_advance);
    RUN_TEST(test_buf_reader_peek_n_beyond_content_returns_minus_one);
    RUN_TEST(test_buf_reader_peek_n_exceeds_buffer_returns_minus_one);

    RUN_TEST(test_buf_reader_consume_null_is_safe);
    RUN_TEST(test_buf_reader_consume_advances_position);

    RUN_TEST(test_buf_reader_is_open_null_returns_false);
    RUN_TEST(test_buf_reader_is_open_before_open_returns_false);

    RUN_TEST(test_buf_reader_has_next_null_returns_false);
    RUN_TEST(test_buf_reader_has_next_true_before_eof);
    RUN_TEST(test_buf_reader_has_next_false_after_eof);

    RUN_TEST(test_buf_reader_has_error_null_returns_false);
    RUN_TEST(test_buf_reader_has_error_false_on_clean_reader);
    RUN_TEST(test_buf_reader_has_error_true_after_missing_file);

    return UNITY_END();
}