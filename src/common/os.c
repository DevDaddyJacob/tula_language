#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"
#include "strings.h"

#ifdef OS_WINDOWS
	#include <windows.h>
#endif /* OS_WINDOWS */

#ifdef OS_POSIX_COMPLIANT
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif /* OS_POSIX_COMPLIANT */

#ifdef OS_MAC
#endif /* OS_MAC */

#ifdef OS_LINUX
#endif /* OS_LINUX */

#ifdef OS_UNIX
#endif /* OS_UNIX */

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

static const char* path_find_last_separator(const char* path);



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

static const char* path_find_last_separator(const char* path) {
#if !defined(OS_WINDOWS) && !defined(OS_POSIX_COMPLIANT)
#error Unsupported platform, \
	no compliant implementation of function 'path_find_last_separator'.
#endif

#ifdef OS_WINDOWS
	const char* lastSlash = strrchr(path, '/');
	const char* lastBackslash = strrchr(path, '\\');

	if (!lastSlash)
	{
		return lastBackslash;
	}

	if (!lastBackslash)
	{
		return lastSlash;
	}

	if (lastSlash > lastBackslash)
	{
		return lastSlash;
	}

	return lastBackslash;
#endif /* OS_WINDOWS */

#ifdef OS_POSIX_COMPLIANT
	return strrchr(path, OS_PATH_SEPARATOR);
#endif /* OS_POSIX_COMPLIANT */
}


char* os_path_normalize(const char* path) {
#if !defined(OS_WINDOWS) && !defined(OS_POSIX_COMPLIANT)
#error Unsupported platform, \
	no compliant implementation of function 'os_path_normalize'.
#endif

    /* Validate the path */
    if (NULL == path)
    {
	    return NULL;
    }


    /* Allocate memory for the new normalized path */
    char* tempPath = NULL;
    tempPath = (char*)malloc(sizeof(char) * OS_MAX_PATH_LENGTH);
    if (tempPath == NULL)
    {
        return NULL;
    }

    str_copy_safe(tempPath, path, OS_MAX_PATH_LENGTH);


    /* Make the separators consistent */
	for (char* c = tempPath; *c; c++)
	{
#if defined(OS_WINDOWS)
        if ('/' == *c)
#elif defined(OS_POSIX_COMPLIANT)
        if ('\\' == *c)
#endif
        {
        	*c = OS_PATH_SEPARATOR;
        }
    }


    /* Replace any '.' and '..' if it's not the first character */
    uint8_t isFirstToken = 1;
    int32_t top = -1;
    const char PATH_SEP_STR[2] = { OS_PATH_SEPARATOR, '\0' };
    char* parts[OS_MAX_PATH_LENGTH];
    char* token = strtok(tempPath, PATH_SEP_STR);

    while (NULL != token)
    {
        /* Check if the token is "." and not the first token */
        if (str_equals(token, ".", 1))
        {
            if (isFirstToken)
            {
            	parts[++top] = token;
            }
        }
    	else if (str_equals(token, "..", 2))
        {
            if (top >= 0)
            {
            	top--;
            }
        }
    	else
        {
            parts[++top] = token;
        }

        token = strtok(NULL, PATH_SEP_STR);
        isFirstToken = 0;
    }


    /* Rebuild the path parts */
    char rebuiltPath[OS_MAX_PATH_LENGTH] = "";
    for (int32_t i = 0; i <= top; i++)
    {
        if (0 == i)
        {
        	strcat(rebuiltPath, "");
        }
    	else
    	{
        	strcat(rebuiltPath, PATH_SEP_STR);
    	}

        strcat(rebuiltPath, parts[i]);
    }


    /* If the rebuilt string is empty, return the "." path */
    if ('\0' == rebuiltPath[0])
    {
    	str_copy_safe(rebuiltPath, ".", OS_MAX_PATH_LENGTH);
    }


    /* Allocate memory for the new normalized path */
    const size_t normalizedPathLen = strlen(rebuiltPath) + 1;
    char* normalizedPath = NULL;

    normalizedPath = (char*)malloc(sizeof(char) * normalizedPathLen);
    if (normalizedPath == NULL)
    {
        return NULL;
    }


    /* Copy the rebuilt path into the normalized one and resize the memory */
    str_copy_safe(normalizedPath, rebuiltPath, normalizedPathLen);

    return normalizedPath;
}


char* os_path_basename(const char* path)  {
	/* Validate the path */
	if (NULL == path)
	{
		return NULL;
	}


	/* Normalize the path */
	char* nPath = os_path_normalize(path);


	/* Find the last separator char */
	const char* lastSeparator = path_find_last_separator(nPath);


	/* If no path separator exists, return standard */
	if (!lastSeparator)
	{
		char* baseName = malloc(sizeof(char) * 2);
		if (NULL == baseName)
		{
			free(nPath);
			return NULL;
		}

		str_copy_safe(baseName, ".", 2);

		free(nPath);
		return baseName;
	}


	/* Determine the length of the base name */
	const size_t length = lastSeparator - nPath;
	if (length <= 0)
	{
		free(nPath);
		return NULL;
	}


	/* Create allocation for the base name and copy into it */
	char* baseName = malloc(sizeof(char) * (length - 1));
	if (NULL == baseName)
	{
		free(nPath);
		return NULL;
	}

	str_copy_safe(baseName, lastSeparator + 1, length - 1);

	free(nPath);
	return baseName;
}


bool os_path_exists(const char* path) {
#if !defined(OS_WINDOWS) && !defined(OS_POSIX_COMPLIANT)
#error Unsupported platform, \
	no compliant implementation of function 'os_path_exists'.
#endif

	/* Validate the path */
	if (NULL == path)
	{
		return false;
	}


	/* Normalize the path */
	char* nPath = os_path_normalize(path);


	/* Perform platform dependant check */
#if defined(OS_WINDOWS)
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(nPath))
#elif defined(OS_POSIX_COMPLIANT)
	if (0 != stat(nPath, NULL))
#endif
	{
		free(nPath);
		return false;
	}

	free(nPath);
	return true;
}


bool os_path_is_file(const char* path) {
#if !defined(OS_WINDOWS) && !defined(OS_POSIX_COMPLIANT)
#error Unsupported platform, \
	no compliant implementation of function 'os_path_is_file'.
#endif

	/* Validate the path */
	if (NULL == path)
	{
		return false;
	}


	/* Normalize the path */
	char* nPath = os_path_normalize(path);


	/* Perform platform dependant check */
#if defined(OS_WINDOWS)
	/* Get the file attributes */
	const DWORD fileAttributes = GetFileAttributesA(nPath);
	if (INVALID_FILE_ATTRIBUTES == fileAttributes) {
		free(nPath);
		return false;
	}


	/* Ensure the path isn't a directory */
	if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		free(nPath);
		return false;
	}
#endif

#if defined(OS_POSIX_COMPLIANT)
	/* Get the file stat */
	struct stat fileStat;
	if (0 != stat(nPath, &fileStat)) {
		free(nPath);
		return false;
	}


	/* Ensure the path is a file */
	if (!S_ISREG(fileStat.st_mode)) {
		free(nPath);
		return false;
	}

#endif

	free(nPath);
	return true;
}
