#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifndef __ASSEMBLY__

#if __GNUC__ || __clang__

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define UNUSED          __attribute__((__unused__))
#define USED            __attribute__((__used__))

#define PACKED          __attribute__((packed))
#define ALIGNED(x)      __attribute__((aligned(x)))

#define SECTION(x)      USED __attribute__((section(x)))

#define NOINLINE        __attribute__((noinline))
#define ALWAYS_INLINE   inline __attribute__((always_inline))

#define NORETURN        __attribute__((noreturn))
#define MALLOC          __attribute__((malloc))
#define WEAK            __attribute__((weak))

#define ISCONSTANT(x)   __builtin_constant_p(x)

#define CONSTRUCTOR     __attribute__((constructor))
#define DESTRUCTOR      __attribute__((destructor))

#define PRINTFLIKE(__fmt, __varargs)    __attribute__((__format__(__printf__, __fmt, __varargs)))

#endif /* __GNUC__ || __clang__ */

#endif /* __ASSEMBLY__ */

#endif /* _COMPILER_H_ */