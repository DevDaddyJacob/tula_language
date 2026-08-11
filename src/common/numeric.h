#ifndef TULA_COMMON_NUMERIC_H
#define TULA_COMMON_NUMERIC_H

#include <stdint.h>

typedef uint8_t							numeric_conversion_status_t;
typedef numeric_conversion_status_t		num_conv_status_t;


#define _NUM_CONV_STATUS(val)	((numeric_conversion_status_t)val)				/* NOLINT(*-reserved-identifier) */

#define NUM_CONV_OK				_NUM_CONV_STATUS(0)
#define NUM_CONV_INVALID		_NUM_CONV_STATUS(1)
#define NUM_CONV_EXCEEDS_RANGE	_NUM_CONV_STATUS(2)
#define NUM_CONV_EXCEEDS_MIN	_NUM_CONV_STATUS(3)
#define NUM_CONV_EXCEEDS_MAX	_NUM_CONV_STATUS(4)


num_conv_status_t str_to_int8(const char* in, int8_t* out);

num_conv_status_t str_to_uint8(const char* in, uint8_t* out);

num_conv_status_t str_to_int16(const char* in, int16_t* out);

num_conv_status_t str_to_uint16(const char* in, uint16_t* out);

num_conv_status_t str_to_int32(const char* in, int32_t* out);

num_conv_status_t str_to_uint32(const char* in, uint32_t* out);

num_conv_status_t str_to_int64(const char* in, int64_t* out);

num_conv_status_t str_to_uint64(const char* in, uint64_t* out);

#endif /* TULA_COMMON_NUMERIC_H */
