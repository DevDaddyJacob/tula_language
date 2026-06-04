#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "strings.h"


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

size_t str_copy(char* destination, const char* source, const size_t destSize)
{
    /* Null check our arguments*/
    if (NULL == destination || NULL == source)
    {
    	return 0;
    }


    /* Validate the destination size */
    if (0 >= destSize)
    {
    	return 0;
    }


    /* Copy the characters up to destSize - 1 */
    size_t i = 0;
    for (i = 0; i < destSize - 1 && source[i] != '\0'; i++)
    {
        destination[i] = source[i];
    }


    /* Ensure the last character written is the null terminator */
    destination[i] = '\0';

    return i + 1;
}


bool str_equals(const char* str, const char* target, const size_t targetLen)
{
	/* Handle null comparison */
	if (NULL == str || NULL == target)
	{
		return NULL == str && NULL == target;
	}


	/* Length validation */
	if (0 >= targetLen)
	{
		return false;
	}


    /* Compare the memory of the strings */
    return 0 == memcmp(str, target, targetLen);
}


bool str_partialEquals(
    const char* str,
    const char* target,
    const size_t targetLen,
    const size_t strOffset
) {
    /* Handle null comparison */
    if (NULL == str || NULL == target)
    {
    	return NULL == str && NULL == target;
    }


    /* Length validation */
    if (0 >= strOffset || 0 >= targetLen)
    {
	    return false;
    }


    /* Compare the memory of the strings */
    return 0 == memcmp(str + strOffset, target, targetLen);
}


bool str_startsWith(
    const char* str,
    const char* target,
    const size_t strLen,
    const size_t targetLen
) {
	/* Handle null comparison */
	if (NULL == str || NULL == target)
	{
		return false;
	}


	/* Length validation */
	if (0 >= strLen || 0 >= targetLen)
	{
		return false;
	}


    /* Safety check the lengths */
    if (targetLen > strLen)
    {
    	return false;
    }


    /* Compare the memory of the strings */
    return 0 == memcmp(str, target, targetLen);
}


bool str_endsWith(
    const char* str,
    const char* target,
    const size_t strLen,
    const size_t targetLen
) {
	/* Handle null comparison */
	if (NULL == str || NULL == target)
	{
		return false;
	}


	/* Length validation */
	if (0 >= strLen || 0 >= targetLen)
	{
		return false;
	}


	/* Safety check the lengths */
	if (targetLen > strLen)
	{
		return false;
	}


    /* Compare the memory of the strings */
    return 0 == memcmp(str + (strLen - targetLen), target, targetLen);
}


bool str_startsWithChar(const char* str, const char target) {
    /* Handle null comparison */
    if (str == NULL)
    {
    	return false;
    }


    /* Compare the memory of the strings */
    return str[0] == target;
}


bool str_endsWithChar(
	const char* str,
	const char target,
	const size_t strLen
) {
    /* Handle null comparison */
    if (NULL == str)
    {
    	return false;
    }


    /* Length validation */
    if (0 >= strLen)
    {
    	return false;
    }


    /* Compare the memory of the strings */
    return str[strLen - 1] == target;
}
