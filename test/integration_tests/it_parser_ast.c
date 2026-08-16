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

DEFINE_TULAD_TEST_PARSER(comparison)
DEFINE_TULAD_TEST_PARSER(functions)
DEFINE_TULAD_TEST_PARSER(iteration)
DEFINE_TULAD_TEST_PARSER(variables)

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_parser_comparison);
	RUN_TEST(test_parser_functions);
	RUN_TEST(test_parser_iteration);
	RUN_TEST(test_parser_variables);

	return UNITY_END();
}
