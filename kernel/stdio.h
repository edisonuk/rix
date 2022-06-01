#ifndef _STD_IO_H_
#define _STD_IO_H_

#include "compiler.h"
#include <stdarg.h>

int printf(const char* fmt, ...) PRINTFLIKE(1, 2);
int vprintf(const char* fmt, va_list ap);

#endif