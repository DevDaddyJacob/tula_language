#ifndef TULA_INTEGRATION_TEST_HELPER_H
#define TULA_INTEGRATION_TEST_HELPER_H

#ifndef IS_OS_DEFINED
#	undef OS_WINDOWS
#	undef OS_MAC
#	undef OS_UNIX
#	undef OS_POSIX_COMPLIANT

#	if defined(_WIN32) || defined(_WIN64)
#		define OS_WINDOWS
#		define IS_OS_DEFINED
#	elif defined(__APPLE__) && defined(__MACH__)
#		define OS_MAC
#		define OS_POSIX_COMPLIANT
#		define IS_OS_DEFINED
#	elif defined(__linux__) || defined(__unix__)
#		define OS_UNIX
#		define OS_POSIX_COMPLIANT
#		define IS_OS_DEFINED
#	else
#		error "Unknown or unsupported platform"
#	endif
#endif /* IS_OS_DEFINED */

#include <stdint.h>
#include <stdio.h>

#ifdef OS_WINDOWS
#	define os_popen(p0, p1) (_popen(p0, p1))
#	define os_pclose(p0) (_pclose(p0))
#endif /* OS_WINDOWS */

#ifdef OS_POSIX_COMPLIANT
#	define os_popen(p0, p1) (popen(p0, p1))
#	define os_pclose(p0) (pclose(p0))
#endif /* OS_POSIX_COMPLIANT */

/* 5 KB */
#define STDOUT_BUFF_SIZE 5120



typedef struct tula_dynamic_buffer
{
	size_t count;
	size_t capacity;
	char* values;
} dynamic_buffer_t;

void dynamic_buffer_init(dynamic_buffer_t* buffer);
void dynamic_buffer_destroy(dynamic_buffer_t* buffer);
void dynamic_buffer_write(
	dynamic_buffer_t* buffer,
	const char* content,
	size_t contentSize
);


void read_file(const char* path, dynamic_buffer_t* dBuffer);


void execute_tulad(
	const char* args,
	dynamic_buffer_t* outStdoutBuff,
	int32_t* outStatus
);

#endif /* TULA_INTEGRATION_TEST_HELPER_H */
