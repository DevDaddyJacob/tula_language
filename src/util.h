#ifndef TULA_LANGUAGE_UTIL_H
#define TULA_LANGUAGE_UTIL_H

#include <stdarg.h>
#include "tula.h"

#define PRINT_FAILED (-1)
#define PRINT_PARTIAL_SUCCESS (-2)


int32_t errPrintF(const char* format, ...);

int32_t vErrPrint(const char* format, va_list vargs);

#endif /* TULA_LANGUAGE_UTIL_H */
