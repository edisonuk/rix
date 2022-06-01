#ifndef _PRINTF_CORE_H_
#define _PRINTF_CORE_H_

#include "compiler.h"

#include <stdarg.h>

int printf_core(const char *fmt, va_list ap);

#endif /* _PRINTF_CORE_H_ */