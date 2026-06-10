#include "unity.h"
#include "unity_memory.h"
#include "common/strings.h"

void setUp(void)
{
    UnityMalloc_StartTest();
}

void tearDown(void)
{
	UnityMalloc_EndTest();
}


/* -------------------------------------------------------------------------
 * char_is_alpha
 * ------------------------------------------------------------------------- */

void test_char_is_alpha_lowercase(void) {
    TEST_ASSERT_TRUE(char_is_alpha('a'));
    TEST_ASSERT_TRUE(char_is_alpha('z'));
    TEST_ASSERT_TRUE(char_is_alpha('m'));
}

void test_char_is_alpha_uppercase(void) {
    TEST_ASSERT_TRUE(char_is_alpha('A'));
    TEST_ASSERT_TRUE(char_is_alpha('Z'));
    TEST_ASSERT_TRUE(char_is_alpha('M'));
}

void test_char_is_alpha_rejects_digits(void) {
    TEST_ASSERT_FALSE(char_is_alpha('0'));
    TEST_ASSERT_FALSE(char_is_alpha('9'));
}

void test_char_is_alpha_rejects_symbols(void) {
    TEST_ASSERT_FALSE(char_is_alpha('!'));
    TEST_ASSERT_FALSE(char_is_alpha(' '));
    TEST_ASSERT_FALSE(char_is_alpha('_'));
}


/* -------------------------------------------------------------------------
 * char_is_digit
 * ------------------------------------------------------------------------- */

void test_char_is_digit_all_digits(void) {
    TEST_ASSERT_TRUE(char_is_digit('0'));
    TEST_ASSERT_TRUE(char_is_digit('5'));
    TEST_ASSERT_TRUE(char_is_digit('9'));
}

void test_char_is_digit_rejects_alpha(void) {
    TEST_ASSERT_FALSE(char_is_digit('a'));
    TEST_ASSERT_FALSE(char_is_digit('Z'));
}

void test_char_is_digit_rejects_symbols(void) {
    TEST_ASSERT_FALSE(char_is_digit('/'));  /* one below '0' */
    TEST_ASSERT_FALSE(char_is_digit(':'));  /* one above '9' */
}


/* -------------------------------------------------------------------------
 * str_copy_safe
 * ------------------------------------------------------------------------- */

void test_str_copy_safe_normal_copy(void) {
    char dest[16];
    const size_t n = str_copy_safe(dest, "hello", sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("hello", dest);
    TEST_ASSERT_EQUAL_size_t(6, n); /* 5 chars + null terminator */
}

void test_str_copy_safe_truncates_to_buffer(void) {
    char dest[4];
    str_copy_safe(dest, "hello", sizeof(dest));
    /* Must always be null-terminated */
    TEST_ASSERT_EQUAL_CHAR('\0', dest[3]);
    /* First 3 chars should be copied */
    TEST_ASSERT_EQUAL_CHAR('h', dest[0]);
    TEST_ASSERT_EQUAL_CHAR('e', dest[1]);
    TEST_ASSERT_EQUAL_CHAR('l', dest[2]);
}

void test_str_copy_safe_empty_source(void) {
    char dest[8] = "garbage";
    const size_t n = str_copy_safe(dest, "", sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("", dest);
    TEST_ASSERT_EQUAL_size_t(1, n); /* just the null terminator */
}

void test_str_copy_safe_exact_fit(void) {
    char dest[6];
    const size_t n = str_copy_safe(dest, "hello", sizeof(dest));
    TEST_ASSERT_EQUAL_STRING("hello", dest);
    TEST_ASSERT_EQUAL_size_t(6, n);
}


/* -------------------------------------------------------------------------
 * str_equals
 * ------------------------------------------------------------------------- */

void test_str_equals_identical_strings(void) {
    TEST_ASSERT_TRUE(str_equals("hello", "hello", 5));
}

void test_str_equals_different_strings(void) {
    TEST_ASSERT_FALSE(str_equals("hello", "world", 5));
}

void test_str_equals_empty_strings(void) {
    TEST_ASSERT_FALSE(str_equals("", "", 0));
}

void test_str_equals_case_sensitive(void) {
    TEST_ASSERT_FALSE(str_equals("Hello", "hello", 5));
}

void test_str_equals_prefix_is_not_equal(void) {
    /* "hell" should not equal "hello" when targetLen is 5 */
    TEST_ASSERT_FALSE(str_equals("hell", "hello", 5));
}


/* -------------------------------------------------------------------------
 * str_equals_partial
 * ------------------------------------------------------------------------- */

void test_str_equals_partial_match_at_offset(void) {
    TEST_ASSERT_TRUE(str_equals_partial("say hello", "hello", 5, 4));
}

void test_str_equals_partial_zero_offset(void) {
    TEST_ASSERT_TRUE(str_equals_partial("hello world", "hello", 5, 0));
}

void test_str_equals_partial_no_match(void) {
    TEST_ASSERT_FALSE(str_equals_partial("say hello", "world", 5, 4));
}

void test_str_equals_partial_offset_past_match(void) {
    TEST_ASSERT_FALSE(str_equals_partial("say hello", "hello", 5, 5));
}


/* -------------------------------------------------------------------------
 * str_starts_with
 * ------------------------------------------------------------------------- */

void test_str_starts_with_matching_prefix(void) {
    TEST_ASSERT_TRUE(str_starts_with("hello world", "hello", 11, 5));
}

void test_str_starts_with_full_match(void) {
    TEST_ASSERT_TRUE(str_starts_with("hello", "hello", 5, 5));
}

void test_str_starts_with_no_match(void) {
    TEST_ASSERT_FALSE(str_starts_with("hello world", "world", 11, 5));
}

void test_str_starts_with_target_longer_than_str(void) {
    TEST_ASSERT_FALSE(str_starts_with("hi", "hello", 2, 5));
}

void test_str_starts_with_empty_target(void) {
    TEST_ASSERT_FALSE(str_starts_with("hello", "", 5, 0));
}


/* -------------------------------------------------------------------------
 * str_ends_with
 * ------------------------------------------------------------------------- */

void test_str_ends_with_matching_suffix(void) {
    TEST_ASSERT_TRUE(str_ends_with("hello world", "world", 11, 5));
}

void test_str_ends_with_full_match(void) {
    TEST_ASSERT_TRUE(str_ends_with("hello", "hello", 5, 5));
}

void test_str_ends_with_no_match(void) {
    TEST_ASSERT_FALSE(str_ends_with("hello world", "hello", 11, 5));
}

void test_str_ends_with_target_longer_than_str(void) {
    TEST_ASSERT_FALSE(str_ends_with("hi", "hello", 2, 5));
}

void test_str_ends_with_empty_target(void) {
    TEST_ASSERT_FALSE(str_ends_with("hello", "", 5, 0));
}


/* -------------------------------------------------------------------------
 * str_starts_with_char
 * ------------------------------------------------------------------------- */

void test_str_starts_with_char_match(void) {
    TEST_ASSERT_TRUE(str_starts_with_char("hello", 'h'));
}

void test_str_starts_with_char_no_match(void) {
    TEST_ASSERT_FALSE(str_starts_with_char("hello", 'e'));
}

void test_str_starts_with_char_empty_string(void) {
    TEST_ASSERT_FALSE(str_starts_with_char("", 'h'));
}


/* -------------------------------------------------------------------------
 * str_ends_with_char
 * ------------------------------------------------------------------------- */

void test_str_ends_with_char_match(void) {
    TEST_ASSERT_TRUE(str_ends_with_char("hello", 'o', 5));
}

void test_str_ends_with_char_no_match(void) {
    TEST_ASSERT_FALSE(str_ends_with_char("hello", 'h', 5));
}

void test_str_ends_with_char_single_char_string(void) {
    TEST_ASSERT_TRUE(str_ends_with_char("x", 'x', 1));
}

void test_str_ends_with_char_empty_string(void) {
    TEST_ASSERT_FALSE(str_ends_with_char("", 'x', 0));
}


/* -------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_char_is_alpha_lowercase);
    RUN_TEST(test_char_is_alpha_uppercase);
    RUN_TEST(test_char_is_alpha_rejects_digits);
    RUN_TEST(test_char_is_alpha_rejects_symbols);

    RUN_TEST(test_char_is_digit_all_digits);
    RUN_TEST(test_char_is_digit_rejects_alpha);
    RUN_TEST(test_char_is_digit_rejects_symbols);

    RUN_TEST(test_str_copy_safe_normal_copy);
    RUN_TEST(test_str_copy_safe_truncates_to_buffer);
    RUN_TEST(test_str_copy_safe_empty_source);
    RUN_TEST(test_str_copy_safe_exact_fit);

    RUN_TEST(test_str_equals_identical_strings);
    RUN_TEST(test_str_equals_different_strings);
    RUN_TEST(test_str_equals_empty_strings);
    RUN_TEST(test_str_equals_case_sensitive);
    RUN_TEST(test_str_equals_prefix_is_not_equal);

    RUN_TEST(test_str_equals_partial_match_at_offset);
    RUN_TEST(test_str_equals_partial_zero_offset);
    RUN_TEST(test_str_equals_partial_no_match);
    RUN_TEST(test_str_equals_partial_offset_past_match);

    RUN_TEST(test_str_starts_with_matching_prefix);
    RUN_TEST(test_str_starts_with_full_match);
    RUN_TEST(test_str_starts_with_no_match);
    RUN_TEST(test_str_starts_with_target_longer_than_str);
    RUN_TEST(test_str_starts_with_empty_target);

    RUN_TEST(test_str_ends_with_matching_suffix);
    RUN_TEST(test_str_ends_with_full_match);
    RUN_TEST(test_str_ends_with_no_match);
    RUN_TEST(test_str_ends_with_target_longer_than_str);
    RUN_TEST(test_str_ends_with_empty_target);

    RUN_TEST(test_str_starts_with_char_match);
    RUN_TEST(test_str_starts_with_char_no_match);
    RUN_TEST(test_str_starts_with_char_empty_string);

    RUN_TEST(test_str_ends_with_char_match);
    RUN_TEST(test_str_ends_with_char_no_match);
    RUN_TEST(test_str_ends_with_char_single_char_string);
    RUN_TEST(test_str_ends_with_char_empty_string);

    return UNITY_END();
}