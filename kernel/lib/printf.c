#include "../printf.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define LONG_FLAG           0x001
#define LONGLONG_FLAG       0x002
#define SIGNED_FLAG         0x004
#define LEADINGZERO_FLAG    0x008
#define LEFTFORMAT_FLAG     0x010
#define SHOWSIGN_FLAG       0x020

NOINLINE static char *longlong_to_string(char *buf, size_t len, unsigned long long ll,
                                    uint32_t flags, char *signchar) {
    size_t pos = len;
    bool negative = 0;
    
    if ((flags & SIGNED_FLAG) && (long long)ll < 0) {
        negative = true;
        ll = -ll;
    }

    buf[--pos] = 0;

    if (negative) {
        *signchar = '-';
    } else if (flags & SHOWSIGN_FLAG) {
        *signchar = '+';
    } else {
        *signchar = ' ';
    }
}

union double_int {
    double d;
    uint64_t i;
};

#define OUT_STR(str)                            \
    do {                                        \
        for (size_t i = 0; str[i] != 0; ++i) {  \
            buf[++pos] = (str)[i];              \
        }                                       \
    } while(0)

/* IEEE 754 Double-precision FP */
NOINLINE static char *double_to_string(char *buf, size_t len, double d, uint32_t flags) {
    size_t pos = 0;

    union double_int du = { d };

    uint32_t exponent = (du.i >> 52) & 0x7ff;
    uint32_t fraction = (du.i & ((1ULL << 52) - 1));
    bool negative = !!(du.i & (1ULL << 63));

    if (negative) {
        buf[++pos] = '-';
        d = -d;
    }

    if (exponent == 0x7ff) {
        if (fraction == 0) {
            OUT_STR("inf");
        } else {
            OUT_STR("nan");
        }
    } else if (exponent == 0) {
        if (fraction == 0) {
            OUT_STR("0.000000");
        } else {
            /* denormalized */
            OUT_STR("den");
        }
    } else {
        int exponent_signed = exponent - 1023;
        if (exponent_signed < -52 || exponent_signed > 52) {
            OUT_STR("<range>");
        } else {  
/* reverse output */
#define OUT_REVERSE(ch)                     \
    do {                                    \
        if (&buf[pos] == buf) goto done;    \
        else buf[--pos] = ch;               \
    } while (0)

            /* start by walking backwards through the string */
            pos = len;
            OUT_REVERSE('0');

            uint32_t i;
            for (i = 0; i <= 6; ++i)
                OUT_REVERSE('0');

            size_t decimal_pos = pos;

            /* integral portion */
            uint64_t u;
            if (exponent_signed >= 0) {
                u = fraction;
                u |= (1ULL << 52);
                u >>= (52 - exponent_signed);

                char *s = longlong_to_string(buf, pos + 1, u, flags, &(char) {0});

                pos = s - buf;
            } else {
                u = 0;
                OUT_REVERSE('0');
            }

            buf[decimal_pos] = '.';

            /* fractional portion */
            uint32_t frac = ((d - u) * 1000000) + 0.5;

            i = decimal_pos + 6 + 1;
            while (frac != 0) {
                uint32_t digit = frac % 10;
                buf[--i] = digit + '0';
                frac /= 10;
            }

            if (negative)
                OUT_REVERSE('-');
done:
            return &buf[pos];
        }
#undef OUT_REVERSE
    }

    buf[pos] = 0;
    return buf;
}

NOINLINE static char *double_to_hexstring(char *buf, size_t len, double d, uint32_t flags) {
    size_t pos = 0;

    
}

#undef OUT_STR

int printf_core(const char *fmt, va_list ap) {
    int ret = 0;

    char c;
    const char *s;
    size_t str_len;
    
    unsigned int format_num;
    int flags;

    char sign_char;
    
    while (true) {
        format_num = 0;
        sign_char = '\0';

        /* handle regular chars that aren't format related */
        s = fmt;
        str_len = 0;
        while ((c = *fmt++)) {
            if (c == '%')
                break;
            str_len++;
        }
        
next:
        c = *fmt++;
        if (c == 0)
            break;

        switch (c) {
        case '0'...'9':
            format_num *= 10;
            format_num += c - '0';
            goto next;

        case 's':
            s = va_arg(ap, const char *);
            if (s == 0)
                s = "<null>";

        case 'F':
        case 'f':
            double d = va_arg(ap, double);
            s = double_to_string();

        default:
            break;
        }

        continue;

out_string:
        str_len = strlen(s);

        if (flags & LEFTFORMAT_FLAG) {
            /* left justify the text */
            OUTPUT_STRING(s, str_len);
            uint32_t written = ret;

            /* padd to right if necessary */
            for (; format_num > written; --format_num)
                OUTPUT_CHAR(' ');
        } else {
            /* right justify the text */
            if (flags & LEADINGZERO_FLAG && sign_char != '\0')
                OUTPUT_CHAR(sign_char);

            /* pad according to the format string */
            for (; format_num > str_len; --format_num)
                OUTPUT_CHAR(flags & LEADINGZERO_FLAG ? '0' : ' ');

            /* if not leading zeros, output the sign char just before the number */
            if (!(flags & LEADINGZERO_FLAG))
                OUTPUT_CHAR();
        }
    }
}