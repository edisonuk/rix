#ifndef _ASM_H_
#define _ASM_H_

#ifndef __ASSEMBLER__
#error "For assembly files only"
#endif

#define ELF_FUNC(x)         .type x, @function
#define ELF_SIZE(x,s)       .size x, s

#define BEGIN_FUNCTION(x)   \
    .global x;              \
    ELF_FUNC(x);            \
    x:

/* Calculate the size of the function label */
#define END_FUNCTION(x)     ELF_SIZE(x, . - x)

#define ELF_DATA(x)         .type x, @object
#define DATA(x)             ELF_DATA(x)

#endif /* _ASM_H_ */