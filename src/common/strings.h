#ifndef TULA_COMMON_STRINGS_H
#define TULA_COMMON_STRINGS_H

#include <stdbool.h>
#include <stdint.h>

#define char_is_alpha(c) \
	(('a' <= c && 'z' >= c) || ('A' <= c && 'Z' >= c))

#define char_is_digit(c) \
	('0' <= c && '9' >= c)

#define char_is_whitespace(c) \
	(' ' == c || '\t' == c || '\n' == c || '\r' == c || '\0' == c)


/**
 * \brief           Safely copies a string into a destination buffer, at most
 *                  copying size - 1 characters and always ensuring a null
 *                  termination
 * \param[out]      destination: The destination buffer to write into
 * \param[in]       source: The source to copy into the destination
 * \param[in]       destSize: The size of the destination buffer
 * \return          Returns the total number of characters copied including
 *                  the null terminator
 */
size_t str_copy_safe(char* destination, const char* source, size_t destSize);


/**
 * \brief           Compares 2 strings for equality
 * \param[in]       str: The string to search in
 * \param[in]       target: The string to search for
 * \param[in]       targetLen: The number of characters in the target string
 * \return          Returns a 1 if the target matches the str, otherwise 0.
 * \note            Return value is a boolean-treated integer
 */
bool str_equals(const char* str, const char* target, size_t targetLen);


bool str_equals_partial(
	const char* str,
	const char* target,
	size_t targetLen,
	size_t strOffset
);


bool str_starts_with(
	const char* str,
	const char* target,
	size_t strLen,
	size_t targetLen
);


bool str_ends_with(
	const char* str,
	const char* target,
	size_t strLen,
	size_t targetLen
);


bool str_starts_with_char(const char* str, char target);


bool str_ends_with_char(const char* str, char target, size_t strLen);

#endif /* TULA_COMMON_STRINGS_H */
