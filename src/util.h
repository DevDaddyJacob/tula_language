#ifndef TULA_LANGUAGE_UTIL_H
#define TULA_LANGUAGE_UTIL_H

#include <stdint.h>

#include "config.h"

/*
 * ============================================================================
 * Common / Utility Macros
 * ============================================================================
 */

#if defined(COMPILER_MSVC)
#	define UNREACHABLE_HINT            __assume(0)
#	define UNREACHABLE_RETURN(value)   __assume(0)
#	define UNREACHABLE_DEFAULT(value)  default: __assume(0)

#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#	define UNREACHABLE_HINT            __builtin_unreachable()
#	define UNREACHABLE_RETURN(value)   __builtin_unreachable()
#	define UNREACHABLE_DEFAULT(value)  default: __builtin_unreachable()

#else
#	define UNREACHABLE_HINT            ((void)0)
#	define UNREACHABLE_RETURN(value)   return (value)
#	define UNREACHABLE_DEFAULT(value)  default: value

#endif

#define UNREACHABLE(value) UNREACHABLE_HINT

#define UNUSED(value) (void)(value)

#define DEFAULT_BREAK default: { break; }

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define NO_RETURN __declspec(noreturn)

/* ========================================================================= */


/*
 * ============================================================================
 * Console Printing
 * ============================================================================
 */

#define PRINT_FAILED (-1)
#define PRINT_PARTIAL_SUCCESS (-2)


int32_t err_print_f(const char* format, ...);

int32_t err_print_v(const char* format, va_list vargs);

/* ========================================================================= */


/*
 * ============================================================================
 * Array interactions
 * ============================================================================
 */

#define tula_array_grow_capacity(capacity) \
	((capacity) < TULA_ARRAY_MIN_THRESHOLD \
		? TULA_ARRAY_MIN_THRESHOLD \
		: (capacity) * TULA_ARRAY_GROW_FACTOR)


#define tula_array_resize(type, pointer, oldCount, newCount) \
	(type*)tula_array_reallocate( \
		pointer, \
		sizeof(type) * (oldCount), \
		sizeof(type) * (newCount) \
	)


#define tula_array_free(type, pointer, oldCount) \
	tula_array_reallocate(pointer, sizeof(type) * (oldCount), 0)



/**
 * \brief           Reallocates an array to be the new size
 * \note            Designed to be used in the grow array & free array macros
 * \param[in]       pointer: Pointer to the array to reallocate
 * \param[in]       oldSize: The old size of the array
 * \param[in]       newSize: The new size of the array
 * \return          Returns a pointer to the new array
 */
void* tula_array_reallocate(void* pointer, size_t oldSize, size_t newSize);

/* ========================================================================= */

#endif /* TULA_LANGUAGE_UTIL_H */
