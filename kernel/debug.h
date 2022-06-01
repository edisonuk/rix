#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "compiler.h"
#include "stdio.h"

#define DEBUG_LEVEL 1   /* ALWAYS */

typedef enum log_level {
    CRITICAL,
    ALWAYS,
    INFO
} log_level_t;

#define debug_printf(level, x...)                   \
    do {                                            \
        if ((level) <= DEBUG_LEVEL) { printf(x); }  \
    } while (0)

/* System-wide halt and prints to log buffer. */
void panic(const char *fmt, ...) PRINTFLIKE(1, 2) NORETURN;

#endif /* _DEBUG_H_ */