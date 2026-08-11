#include "numeric.h"

#include <errno.h>
#include <stdlib.h>

/*
 * ==================================================
 * Macros
 * ==================================================
 */

#define str_to_signed(type, in, out, min, max)									\
	do {																		\
		char* end;																\
																				\
		errno = 0;																\
		const int64_t dirtyValue = strtoll(in, &end, 10);						\
																				\
		if (end == in) {														\
			return NUM_CONV_INVALID;											\
		}																		\
																				\
		if (ERANGE == errno)													\
		{																		\
			return NUM_CONV_EXCEEDS_RANGE;										\
		}																		\
																				\
		if (min > dirtyValue)													\
		{																		\
			return NUM_CONV_EXCEEDS_MIN;										\
		}																		\
																				\
		if (max < dirtyValue)													\
		{																		\
			return NUM_CONV_EXCEEDS_MAX;										\
		}																		\
																				\
		*out = (type)dirtyValue;												\
																				\
		return NUM_CONV_OK;														\
	} while (0);


#define str_to_unsigned(type, in, out, max)										\
	do {																		\
		char* end;																\
																				\
		errno = 0;																\
		const uint64_t dirtyValue = strtoull(in, &end, 10);						\
																				\
		if (end == in) {														\
			return NUM_CONV_INVALID;											\
		}																		\
																				\
		if (ERANGE == errno)													\
		{																		\
			return NUM_CONV_EXCEEDS_RANGE;										\
		}																		\
																				\
		if (0 > dirtyValue)														\
		{																		\
			return NUM_CONV_EXCEEDS_MIN;										\
		}																		\
																				\
		if (max < dirtyValue)													\
		{																		\
			return NUM_CONV_EXCEEDS_MAX;										\
		}																		\
																				\
		*out = (type)dirtyValue;												\
																				\
		return NUM_CONV_OK;														\
	} while (0);

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

num_conv_status_t str_to_int8(const char* in, int8_t* out)
{
	str_to_signed(int8_t, in, out, INT8_MIN, INT8_MAX);
}


num_conv_status_t str_to_uint8(const char* in, uint8_t* out)
{
	str_to_unsigned(uint8_t, in, out, UINT8_MAX);
}


num_conv_status_t str_to_int16(const char* in, int16_t* out)
{
	str_to_signed(int16_t, in, out, INT16_MIN, INT16_MAX);
}


num_conv_status_t str_to_uint16(const char* in, uint16_t* out)
{
	str_to_unsigned(uint16_t, in, out, UINT16_MAX);
}


num_conv_status_t str_to_int32(const char* in, int32_t* out)
{
	str_to_signed(int32_t, in, out, INT32_MIN, INT32_MAX);
}


num_conv_status_t str_to_uint32(const char* in, uint32_t* out)
{
	str_to_unsigned(uint32_t, in, out, UINT32_MAX);
}


num_conv_status_t str_to_int64(const char* in, int64_t* out)
{
	str_to_signed(int64_t, in, out, INT64_MIN, INT64_MAX);
}


num_conv_status_t str_to_uint64(const char* in, uint64_t* out)
{
	str_to_unsigned(uint64_t, in, out, UINT64_MAX);
}
