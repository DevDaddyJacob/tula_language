#include <stdint.h>
#include <stdio.h>

#include "integration_test_helper.h"
#include "unity.h"

void setUp(void) { }

void tearDown(void) { }

void test_scanner_all_tokens(void)
{
	EXECUTE_TULAD_EXPECT_OUTPUT(
		"it_scanner_all_tokens",
		"--mode scanner " \
			"--input-file \"" INTEGRATION_TEST_DATA_DIR "/it_scanner_all_tokens.in\""
	);
}

void test_scanner_sample_1(void)
{
	EXECUTE_TULAD_EXPECT_OUTPUT(
		"it_scanner_sample_1",
		"--mode scanner " \
			"--input-file \"" INTEGRATION_TEST_DATA_DIR "/it_scanner_sample_1.in\""
	);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_scanner_all_tokens);
	RUN_TEST(test_scanner_sample_1);

	return UNITY_END();
}
