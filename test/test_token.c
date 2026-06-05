#include <string.h>

#include "unity.h"
#include "common/strings.h"
#include "engine/scanner/token.h"

void setUp(void) { }

void tearDown(void) { }

void test_token_value_matches(void) {
#define DEFINER(identifier, value, _2, _3, _4, _5, _6) \
	TEST_ASSERT_TRUE( \
		str_equals(TOKENS_VALUE[identifier], value, strlen(value)) \
	);

	DEFINE_TOKENS(DEFINER)
#undef DEFINER
}

void test_token_value_alt_matches(void) {
#define DEFINER(identifier, _1, valueAlt, _3, _4, _5, _6) \
	TEST_ASSERT_TRUE( \
		str_equals( \
			TOKENS_VALUE_ALT[identifier], \
			valueAlt, \
			NULL == valueAlt ? 0 : strlen(valueAlt) \
		) \
	);

	DEFINE_TOKENS(DEFINER)
#undef DEFINER
}

void test_token_is_meta_matches(void) {
#define DEFINER(identifier, _1, _2, isMeta, _4, _5, _6) \
	TEST_ASSERT_TRUE(TOKENS_IS_META[identifier] == isMeta);

	DEFINE_TOKENS(DEFINER)
#undef DEFINER
}

void test_token_is_primitive_matches(void) {
#define DEFINER(identifier, _1, _2, _3, isPrimitive, _5, _6) \
	TEST_ASSERT_TRUE(TOKENS_IS_PRIMITIVE[identifier] == isPrimitive);

	DEFINE_TOKENS(DEFINER)
#undef DEFINER
}

void test_token_is_keyword_matches(void) {
#define DEFINER(identifier, _1, _2, _3, _4, isKeyword, _6) \
	TEST_ASSERT_TRUE(TOKENS_IS_KEYWORD[identifier] == isKeyword);

	DEFINE_TOKENS(DEFINER)
#undef DEFINER
}

void test_token_is_operator_matches(void) {
#define DEFINER(identifier, _1, _2, _3, _4, _5, isOperator) \
	TEST_ASSERT_TRUE(TOKENS_IS_OPERATOR[identifier] == isOperator);

	DEFINE_TOKENS(DEFINER)
#undef DEFINER
}

int main(void) {
    UNITY_BEGIN();

	RUN_TEST(test_token_value_matches);
	RUN_TEST(test_token_value_alt_matches);
	RUN_TEST(test_token_is_meta_matches);
	RUN_TEST(test_token_is_primitive_matches);
    RUN_TEST(test_token_is_keyword_matches);
    RUN_TEST(test_token_is_operator_matches);

    return UNITY_END();
}