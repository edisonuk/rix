#include "../stdio.h"

int printf(const char *fmt, ...) {
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vprintf(fmt, ap);
    va_end(ap);

    return ret;
}

int vprintf(const char* fmt, va_list ap) {
    
}