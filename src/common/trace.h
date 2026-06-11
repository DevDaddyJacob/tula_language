#ifndef TULA_COMMON_TRACE_H
#define TULA_COMMON_TRACE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../config.h"

#ifndef TULA_TRACE_MAX_FRAMES
#  define TULA_TRACE_MAX_FRAMES 64
#endif /* TULA_TRACE_MAX_FRAMES */

#ifndef TULA_TRACE_MAX_SYMB_LEN
#  define TULA_TRACE_MAX_SYMB_LEN 256
#endif /* TULA_TRACE_MAX_SYMB_LEN */

/**
 * \brief	A single captured stack frame.
 */
typedef struct tula_trace_stack_frame {
	/**
	 * \brief	Raw program counter value.
	 */
	void* progCounterAddr;

	/**
	 * \brief	Demangled/resolved name.
	 */
	char symbolName[TULA_TRACE_MAX_SYMB_LEN];

	/**
	 * \brief	Source file (if available).
	 */
	char fileName[TULA_TRACE_MAX_SYMB_LEN];

	/**
	 * \brief	Source line (if available).
	 */
	uint32_t line;
} stack_frame_t;

/**
 * \brief	A captured stack trace.
 */
typedef struct tula_trace_stack_trace {
	/**
	 * \brief	Captured stack frames.
	 */
	stack_frame_t frames[TULA_TRACE_MAX_FRAMES];

	/**
	 * \brief	the number of valid frames.
	 */
	int32_t depth;
} stack_trace_t;


/**
 * \brief	Capture the current call stack into *stackTrace.
 *
 * \param stackTrace[out]	Output structure (must not be NULL).
 * \param skip				Additional frames to skip beyond trace_capture
 *							itself. Pass 0 to start at the caller of
 *							trace_capture.
 */
void trace_capture(stack_trace_t* stackTrace, int32_t skip);


/**
 * \brief	Print a captured stack trace to a FILE stream.
 *
 * \param stackTrace[out]	A trace previously filled by trace_capture().
 * \param destHandle		Destination (e.g. stderr, stdout, or a log file).
 */
void trace_print(const stack_trace_t* stackTrace, FILE* destHandle);


/**
 * \brief	Write a stack trace into a caller-supplied string buffer.
 *
 * \param stackTrace[out]	A trace previously filled by trace_capture().
 * \param buf				Destination buffer.
 * \param bufLen			Size of buf in bytes.
 *
 * \return	Number of characters written (excluding the null terminator), same
 *			semantics as snprintf.
 */
int32_t trace_sprint(const stack_trace_t* stackTrace, char* buf, size_t bufLen);


/**
 * \brief	Convenience: capture and immediately print to fp.
 *
 * \param destHandle	Destination stream.
 * \param skip			Same as trace_capture's skip parameter.
 */
void trace_dump(FILE* destHandle, int32_t skip);

#endif /* TULA_COMMON_TRACE_H */
