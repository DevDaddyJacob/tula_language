#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "tula.h"

/*
 * ==================================================
 * Macros
 * ==================================================
 */

/* #define XYZ "ABC" */

/*
 * ==================================================
 * Typedefs & Prototypes
 * ==================================================
 */

/**
 * TBD
 */
/* static void example(); */


/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

/* int magicNumber = 420; */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

int32_t err_print_f(const char* format, ...) {
	/* Null check the format */
	if (format == NULL) return PRINT_FAILED;


	/* Get the variable arguments */
	va_list vargs;
	va_start(vargs, format);


	/* Send to the v function */
	const int32_t charsPrinted = err_print_v(format, vargs);


	/* Release the vargs and return the status */
	va_end(vargs);

	// ReSharper disable once CppDFAConstantConditions
	if (charsPrinted > TULA_MAX_BUFFER_SIZE)
	{
		/* Don't ask... */
		UNREACHABLE_RETURN(PRINT_PARTIAL_SUCCESS);
	}

	if (charsPrinted <= 0)
	{
		return PRINT_FAILED;
	}

	return charsPrinted;
}


int32_t err_print_v(const char* format, const va_list vargs) {
	/* Null check the format */
	if (format == NULL) return PRINT_FAILED;


	/* Print the message into the program's error format */
	char outFormatBuff[TULA_MAX_BUFFER_SIZE];
	snprintf(
		outFormatBuff,
		TULA_MAX_BUFFER_SIZE,
		TULA_PROGRAM_NAME ": %s\n",
		format
	);


	/* Format the output */
	char outputStr[TULA_MAX_BUFFER_SIZE];
	const int32_t charsPrinted = vsnprintf(
		outputStr,
		TULA_MAX_BUFFER_SIZE,
		outFormatBuff,
		vargs
	);


	/* Print the output to stderr */
	fprintf(stderr, "%s", outputStr);


	/* Return the status */
	if (charsPrinted > TULA_MAX_BUFFER_SIZE)
	{
		return PRINT_PARTIAL_SUCCESS;
	}

	if (charsPrinted <= 0)
	{
		return PRINT_FAILED;
	}

	return charsPrinted;
}