#ifndef TULA_COMMON_OS_H
#define TULA_COMMON_OS_H
#include <stdbool.h>

#ifndef IS_OS_DEFINED // NOLINT(*-reserved-identifier)
	#undef OS_WINDOWS
	#undef OS_MAC
	#undef OS_UNIX
	#undef OS_POSIX_COMPLIANT

	#if defined(_WIN32) || defined(_WIN64)
		#define OS_WINDOWS
		#define IS_OS_DEFINED

	#elif defined(__APPLE__) && defined(__MACH__)
		#define OS_MAC
		#define OS_POSIX_COMPLIANT
		#define IS_OS_DEFINED

	#elif defined(__linux__) || defined(__unix__)
		#define OS_UNIX
		#define OS_POSIX_COMPLIANT
		#define IS_OS_DEFINED

	#else
		#error "Unknown or unsupported platform"
	#endif
#endif /* IS_OS_DEFINED */


#ifndef OS_PATH_SEPARATOR
	#if defined(OS_WINDOWS)
		#define OS_PATH_SEPARATOR '\\'
	#elif defined(OS_POSIX_COMPLIANT)
		#define OS_PATH_SEPARATOR '/'
	#else
		#error "Unsupported platform, " \
		"no compliant implementation of macro 'OS_PATH_SEPARATOR'."
	#endif
#endif /* OS_PATH_SEPARATOR */


#ifndef OS_MAX_PATH_LENGTH
	#if defined(OS_WINDOWS)
		#define OS_MAX_PATH_LENGTH 260
	#elif defined(OS_MAC)
		#define OS_MAX_PATH_LENGTH 1024
	#elif defined(OS_POSIX_COMPLIANT)
		#define OS_MAX_PATH_LENGTH 4096
	#else
		#error "Unsupported platform, " \
			"no compliant implementation of macro 'MAX_PATH_LENGTH'."
	#endif
#endif /* OS_MAX_PATH_LENGTH */


char* os_path_normalize(const char* path);

char* os_path_basename(const char* path);

bool os_path_exists(const char* path);

bool os_path_is_file(const char* path);

#endif /* TULA_COMMON_OS_H */
