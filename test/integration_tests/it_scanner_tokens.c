#include <stdint.h>
#include <stdio.h>

#include "integration_test_helper.h"
#include "unity.h"

void setUp(void) { }

void tearDown(void) { }

void test_scanner_tokens(void)
{
	int32_t status = -1;
	dynamic_buffer_t stdoutBuff;
	dynamic_buffer_t expectedBuff;

	read_file(INTEGRATION_TEST_DIR "/it_scanner_tokens.out", &expectedBuff);

	execute_tulad(
		"--mode scanner " \
			"--input-file \"" INTEGRATION_TEST_DIR "/it_scanner_tokens.in\"",
		&stdoutBuff,
		&status
	);

	TEST_ASSERT_EQUAL_INT32(0, status);
	TEST_ASSERT_EQUAL_STRING(expectedBuff.values, stdoutBuff.values);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_scanner_tokens);

	return UNITY_END();
}
