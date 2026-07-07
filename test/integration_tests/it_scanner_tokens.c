#include <stdint.h>
#include <stdio.h>

#include "integration_test_helper.h"
#include "unity.h"

#define DATA_DIR INTEGRATION_TEST_DATA_DIR "/it_scanner/"

#define DEFINE_TULAD_TEST_SCANNER(name) \
	void test_scanner_##name(void) \
	{ \
		EXECUTE_TULAD_EXPECT_OUTPUT( \
			DATA_DIR #name ".out", \
			"--mode scanner " \
				"--input-file \"" DATA_DIR #name ".in\"" \
		); \
	}


void setUp(void) { }

void tearDown(void) { }

DEFINE_TULAD_TEST_SCANNER(all_tokens)
DEFINE_TULAD_TEST_SCANNER(sample_1)
DEFINE_TULAD_TEST_SCANNER(sample_2)
DEFINE_TULAD_TEST_SCANNER(sample_3)
DEFINE_TULAD_TEST_SCANNER(sample_4)
DEFINE_TULAD_TEST_SCANNER(sample_5)
DEFINE_TULAD_TEST_SCANNER(sample_6)
DEFINE_TULAD_TEST_SCANNER(sample_7)
DEFINE_TULAD_TEST_SCANNER(sample_8)
DEFINE_TULAD_TEST_SCANNER(sample_9)

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_scanner_all_tokens);
	RUN_TEST(test_scanner_sample_1);
	RUN_TEST(test_scanner_sample_2);
	RUN_TEST(test_scanner_sample_3);
	RUN_TEST(test_scanner_sample_4);
	RUN_TEST(test_scanner_sample_5);
	RUN_TEST(test_scanner_sample_6);
	RUN_TEST(test_scanner_sample_7);
	RUN_TEST(test_scanner_sample_8);
	RUN_TEST(test_scanner_sample_9);

	return UNITY_END();
}
