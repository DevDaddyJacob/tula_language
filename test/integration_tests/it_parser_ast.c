#include <stdint.h>
#include <stdio.h>

#include "integration_test_helper.h"
#include "unity.h"

#define DATA_DIR INTEGRATION_TEST_DATA_DIR "/it_parser/"

#define DEFINE_TULAD_TEST_PARSER(name) \
	void test_parser_##name(void) \
	{ \
		EXECUTE_TULAD_EXPECT_OUTPUT( \
			DATA_DIR #name ".out", \
			"--mode parser " \
				"--input-file \"" DATA_DIR #name ".in\"" \
		); \
	}


void setUp(void) { }

void tearDown(void) { }

DEFINE_TULAD_TEST_PARSER(accessors)
DEFINE_TULAD_TEST_PARSER(comparison)
DEFINE_TULAD_TEST_PARSER(control_flow)
DEFINE_TULAD_TEST_PARSER(functions)
DEFINE_TULAD_TEST_PARSER(iteration)
DEFINE_TULAD_TEST_PARSER(literals)
DEFINE_TULAD_TEST_PARSER(numeric_iteration_update)
DEFINE_TULAD_TEST_PARSER(operators)
DEFINE_TULAD_TEST_PARSER(variables)
DEFINE_TULAD_TEST_PARSER(variables_uninitialized)

DEFINE_TULAD_TEST_PARSER(_syntax_err_const_def_missing_equals)
DEFINE_TULAD_TEST_PARSER(_syntax_err_define_unexpected_token)
DEFINE_TULAD_TEST_PARSER(_syntax_err_expected_identifier)
DEFINE_TULAD_TEST_PARSER(_syntax_err_grouping_missing_close_paren)
DEFINE_TULAD_TEST_PARSER(_syntax_err_incdec_target_call)
DEFINE_TULAD_TEST_PARSER(_syntax_err_incdec_target_literal)
DEFINE_TULAD_TEST_PARSER(_syntax_err_missing_close_brace)
DEFINE_TULAD_TEST_PARSER(_syntax_err_missing_open_brace)
DEFINE_TULAD_TEST_PARSER(_syntax_err_set_missing_equals)
DEFINE_TULAD_TEST_PARSER(_syntax_err_set_missing_var_keyword)
DEFINE_TULAD_TEST_PARSER(_syntax_err_unexpected_token)
DEFINE_TULAD_TEST_PARSER(_syntax_err_unterminated_string_leading)
DEFINE_TULAD_TEST_PARSER(_syntax_err_unterminated_string_nested)

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_parser_accessors);
	RUN_TEST(test_parser_comparison);
	RUN_TEST(test_parser_control_flow);
	RUN_TEST(test_parser_functions);
	RUN_TEST(test_parser_iteration);
	RUN_TEST(test_parser_literals);
	RUN_TEST(test_parser_numeric_iteration_update);
	RUN_TEST(test_parser_operators);
	RUN_TEST(test_parser_variables);
	RUN_TEST(test_parser_variables_uninitialized);

	RUN_TEST(test_parser__syntax_err_const_def_missing_equals);
	RUN_TEST(test_parser__syntax_err_define_unexpected_token);
	RUN_TEST(test_parser__syntax_err_expected_identifier);
	RUN_TEST(test_parser__syntax_err_grouping_missing_close_paren);
	RUN_TEST(test_parser__syntax_err_incdec_target_call);
	RUN_TEST(test_parser__syntax_err_incdec_target_literal);
	RUN_TEST(test_parser__syntax_err_missing_close_brace);
	RUN_TEST(test_parser__syntax_err_missing_open_brace);
	RUN_TEST(test_parser__syntax_err_set_missing_equals);
	RUN_TEST(test_parser__syntax_err_set_missing_var_keyword);
	RUN_TEST(test_parser__syntax_err_unexpected_token);
	RUN_TEST(test_parser__syntax_err_unterminated_string_leading);
	RUN_TEST(test_parser__syntax_err_unterminated_string_nested);

	return UNITY_END();
}
