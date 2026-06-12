#ifndef TULA_TEST_H
#define TULA_TEST_H

#include <stdint.h>

#include "config.h"

#ifdef TULA_EXE_DEBUGGING

/*
 * def(
 *		identifier,			<-	The identifier used to define the element
 *		value,				<-	The string of the textual representation of the element
 *	)
 */
#define DEFINE_TEST_MODES(def)			\
	def(TEST_MODE_VERSION,	"version")	\
	def(TEST_MODE_SCANNER,	"scanner")	\

typedef enum tula_test_mode
{
#define TEST_MODE_ENUM_DEFINER(identifier, _1) identifier,
	DEFINE_TEST_MODES(TEST_MODE_ENUM_DEFINER)
#undef TEST_MODE_ENUM_DEFINER
	TOTAL_TEST_MODES
} test_mode_t;

extern const char* TEST_MODE_VALUE[TOTAL_TEST_MODES];


int32_t run_test();

#endif /* TULA_EXE_DEBUGGING */

#endif /* TULA_TEST_H */
