#include "trace.h"

#include "strings.h"
#include "tula.h"

#ifdef OS_POSIX_COMPLIANT
#	include <execinfo.h>
#	include <stdlib.h>

#	ifdef OS_MAC
#		include <dlfcn.h>
#	endif /* OS_MAC */
#endif /* OS_POSIX_COMPLIANT */

#ifdef OS_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif

#	include <windows.h>
#	include <DbgHelp.h>
#endif /* OS_WINDOWS */

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
 * \brief	Format a single frame for output. Returns chars written (snprintf
 *			style).
 */
static int32_t trace_format_frame(
	char* buff,
	size_t len,
	int32_t idx,
	const stack_frame_t* frame
);

#ifdef OS_WINDOWS
static void trace_ensure_init(void);
#endif /* OS_WINDOWS */

/*
 * ==================================================
 * Module Level Variables & Constants
 * ==================================================
 */

#ifdef OS_WINDOWS
/* DbgHelp is not thread-safe; serialize with a critical section. */
static CRITICAL_SECTION trace_dbghelp_critical_section;
static BOOL             trace_dbghelp_init = FALSE;
static HANDLE           trace_process;
#endif /* OS_WINDOWS */


/*
 * ==================================================
 * Function Definitions
 * ==================================================
 */

static int32_t trace_format_frame(
	char* buff,
	const size_t len,
	const int32_t idx,
	const stack_frame_t* frame
)
{
	const char* symbol = "??";
	if (frame->symbolName[0])
	{
		symbol = frame->symbolName;
	}


	if ('\0' != frame->fileName[0] && 0 != frame->line) {
		return snprintf(
			buff,
			len,
			"#%-3d  %p  %s  (%s:%u)\n",
			idx,
			frame->progCounterAddr,
			symbol,
			frame->fileName,
			frame->line
		);
	}

	return snprintf(
		buff,
		len,
		"#%-3d  %p  %s\n",
		idx,
		frame->progCounterAddr,
		symbol
	);
}


#ifdef OS_WINDOWS
static void trace_ensure_init(void)
{
	if (trace_dbghelp_init)
	{
		return;
	}

	InitializeCriticalSection(&trace_dbghelp_critical_section);

	trace_process = GetCurrentProcess();
	SymInitialize(trace_process, NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

	trace_dbghelp_init = TRUE;



	InitializeCriticalSection(&trace_dbghelp_critical_section);
	trace_process = GetCurrentProcess();

	/*
	 * Build a symbol search path of the form:
	 *   <exe_dir>;<exe_dir>;srv*<exe_dir>
	 *
	 * This covers:
	 *   1. PDB sitting next to the .exe (MinGW / MSVC RelWithDebInfo)
	 *   2. DWARF embedded in the binary itself (MinGW -g)
	 *   3. A local symbol server cache in the exe directory
	 *
	 * Passing NULL here makes DbgHelp search only %_NT_SYMBOL_PATH%,
	 * which is almost never set on a developer machine and produces
	 * the "??" output you're seeing.
	 */
	char symbolPath[MAX_PATH * 3];
	char exeDir[MAX_PATH];
	const DWORD len = GetModuleFileNameA(NULL, exeDir, MAX_PATH);

	if (0 < len) {
		/* Strip filename to get directory */
		char *last_sep = exeDir + len;

		while (
			last_sep > exeDir
			&& '\\' != *last_sep
			&& '/' != *last_sep
		)
		{
			--last_sep;
		}
		*last_sep = '\0';

		/* "dir;srv*dir" — local PDB first, then local symbol cache */
		_snprintf(
			symbolPath,
			sizeof(symbolPath),
			"%s;srv*%s",
			exeDir,
			exeDir
		);
	} else {
		symbolPath[0] = '\0';
	}


	SymSetOptions(
		SYMOPT_LOAD_LINES		/* resolve file/line info */
		| SYMOPT_UNDNAME		/* auto-demangle C++ names */
		| SYMOPT_DEFERRED_LOADS	/* load PDBs lazily (faster init) */
	);

	if (symbolPath[0])
	{
		SymInitialize(trace_process, symbolPath, TRUE);
	}
	else
	{
		SymInitialize(trace_process, NULL, TRUE);
	}

	SymInitialize(
		trace_process,
		symbolPath[0] ? symbolPath : NULL,
		TRUE
	);

	trace_dbghelp_init = TRUE;
}
#endif /* OS_WINDOWS */


void trace_capture(stack_trace_t* stackTrace, int skip)
{
	memset(stackTrace, 0, sizeof(*stackTrace));

#if !defined(OS_WINDOWS) && !defined(OS_POSIX_COMPLIANT)
	UNUSED(skip);
#else
	/* +8 for st_capture / st_dump frames */
	void* raw[TULA_TRACE_MAX_SYMB_LEN + 8];
	const int32_t extra = skip + 1;
#endif /* !defined(OS_WINDOWS) && !defined(OS_POSIX_COMPLIANT) */

#ifdef OS_WINDOWS
	/* Symbol buffer – must be large enough for SYMBOL_INFO + name. */
	char symbolsBuff[sizeof(SYMBOL_INFO) + TULA_TRACE_MAX_SYMB_LEN];
	SYMBOL_INFO* symbols = (SYMBOL_INFO*)symbolsBuff;
	IMAGEHLP_LINE64 line;
	DWORD line_disp;

	trace_ensure_init();

	const uint16_t total = CaptureStackBackTrace(
		(ULONG)extra,
		TULA_TRACE_MAX_FRAMES,
		raw,
		NULL
	);


	EnterCriticalSection(&trace_dbghelp_critical_section);

	for (
		int32_t i = 0;
		i < (int)total && stackTrace->depth < TULA_TRACE_MAX_FRAMES;
		++i, ++stackTrace->depth
	)
	{
		const DWORD64 address = (uintptr_t)raw[i];
		stack_frame_t* frame = &stackTrace->frames[stackTrace->depth];
		frame->progCounterAddr = raw[i];


		memset(symbolsBuff, 0, sizeof(symbolsBuff));
		symbols->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbols->MaxNameLen = TULA_TRACE_MAX_SYMB_LEN - 1;

		if (SymFromAddr(trace_process, address, NULL, symbols)) {
			str_copy_safe(
				frame->symbolName,
				symbols->Name,
				TULA_TRACE_MAX_SYMB_LEN
			);
		}


		memset(&line, 0, sizeof(line));
		line.SizeOfStruct = sizeof(line);
		if (SymGetLineFromAddr64(trace_process, address, &line_disp, &line)) {
			str_copy_safe(
				frame->fileName,
				line.FileName,
				TULA_TRACE_MAX_SYMB_LEN
			);

			frame->line = (uint32_t)line.LineNumber;
		}
	}

	LeaveCriticalSection(&trace_dbghelp_critical_section);

#endif /* OS_WINDOWS */

#ifdef OS_POSIX_COMPLIANT
	int32_t total = backtrace(raw, TULA_TRACE_MAX_FRAMES + extra);
	if (total <= extra)
	{
		return;
	}

	char** symbols = backtrace_symbols(raw, total);

	stackTrace->depth = 0;
	for (
		int32_t i = extra;
		i < total && stackTrace->depth < TULA_TRACE_MAX_FRAMES;
		++i, ++stackTrace->depth
	)
	{
		stack_frame_t* frame = &stackTrace->frames[stackTrace->depth];
		frame->progCounterAddr = raw[i];
		frame->line = 0;

		if (symbols && symbols[i])
		{
			str_copy_safe(
				frame->symbolName,
				symbols[i],
				TULA_TRACE_MAX_SYMB_LEN
			);
		}

#if OS_MAC
		/* dladdr gives us the clean symbol name on macOS */
		{
			Dl_info info;
			if (dladdr(raw[i], &info) && info.dli_sname)
			{
				str_copy_safe(
					frame->symbolName,
					info.dli_sname,
					TULA_TRACE_MAX_SYMB_LEN
				);

				if (info.dli_fname)
				{
					str_copy_safe(
						frame->fileName,
						info.dli_fname,
						TULA_TRACE_MAX_SYMB_LEN
					);
				}
			}
		}
#endif /* OS_MAC */
	}

	if (symbols)
	{
		free(symbols);
	}
#endif /* OS_POSIX_COMPLIANT */
}


void trace_print(const stack_trace_t* stackTrace, FILE* destHandle)
{
	if (NULL == destHandle)
	{
		return;
	}

	if (stackTrace->depth == 0) {
		fputs("(no frames captured)\n", destHandle);
		return;
	}

	for (int32_t i = 0; i < stackTrace->depth; ++i) {
		char line[512];
		trace_format_frame(line, sizeof(line), i, &stackTrace->frames[i]);
		fputs(line, destHandle);
	}
}


int32_t trace_sprint(
	const stack_trace_t* stackTrace,
	char* buf,
	size_t bufLen
)
{
	if (NULL == buf || bufLen == 0)
	{
		return 0;
	}

	buf[0] = '\0';

	int32_t n;
	if (0 == stackTrace->depth)
	{
		n = snprintf(buf, bufLen, "(no frames captured)\n");

		if (0 < n)
		{
			return n;
		}

		return 0;
	}


	int  written = 0;
	char line[512];

	for (int32_t i = 0; i < stackTrace->depth; ++i) {
		n = trace_format_frame(line, sizeof(line), i, &stackTrace->frames[i]);
		if (0 >= n)
		{
			continue;
		}

		if ((size_t)written + n < bufLen)
		{
			memcpy(buf + written, line, (size_t)n);
			written += n;
			buf[written] = '\0';
		}
		else
		{
			/* Truncate gracefully */
			const size_t remaining = bufLen - (size_t)written - 1;

			if (0 < remaining) {
				memcpy(buf + written, line, remaining);
				written += (int32_t)remaining;
			}

			buf[written] = '\0';
			break;
		}
	}

	return written;
}


void trace_dump(FILE* destHandle, const int32_t skip)
{
	stack_trace_t stack_trace;
	trace_capture(&stack_trace, skip + 1); /* +1 to skip st_dump itself */

	fputs("\nStack Trace:\n", destHandle);
	trace_print(&stack_trace, destHandle);
}