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

DEFINE_TULAD_TEST_PARSER(variables)
DEFINE_TULAD_TEST_PARSER(control_flow)
DEFINE_TULAD_TEST_PARSER(functions)
DEFINE_TULAD_TEST_PARSER(collections)
DEFINE_TULAD_TEST_PARSER(accessors)
DEFINE_TULAD_TEST_PARSER(syntax_error)

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_parser_variables);
	RUN_TEST(test_parser_control_flow);
	RUN_TEST(test_parser_functions);
	RUN_TEST(test_parser_collections);
	RUN_TEST(test_parser_accessors);
	RUN_TEST(test_parser_syntax_error);

	return UNITY_END();
}
