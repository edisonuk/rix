#ifndef _ASM_H_
#define _ASM_H_

#ifndef __ASSEMBLER__
#error "For assembly files only"
#endif

#define ELF_FUNC(x)         \
    .type x, @function

#define ELF_SIZE(x,s)       \
    .size x, s

#define BEGIN_FUNCTION(x)   \
    .global x;              \
    ELF_FUNC(x);            \
    x:

#define END_FUNCTION(x)     \
    ELF_SIZE(x, . - x)

#define ELF_DATA(x)         \
    .type x, @object

#endif /* _ASM_H_ */