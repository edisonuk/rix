#ifndef _X86_H_
#define _X86_H_

#include <types.h>
#include <reg_defs.h>

static inline void cpuid(uint32_t level, uint32_t *eax, uint32_t *ebx uint32_t *ecx, uint32_t *edx) {
    __asm__ __volatile__(
        "cpuid\n\t",
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "=a"(leaf), 
    );
}

static unsigned long cpuid_get_addr_size(void) {
    uint32_t eax, ebx, ecx, edx;
 
    cpuid(0x80000008, *eax, *ebx, *ecx, *edx);

    /* Bits 07-00: Physical Address bits */
    /* Bits 15-08: Linear Address bits */
    return (eax & 0xff);
}

static inline unsigned long get_cr3(void) {
    unsigned long rv;

    __asm__ __volatile__(
        "mov %%cr0, %0 \n\t"
        : "=r"(rv)
    );

    return rv;
}

static inline void set_cr3(unsigned long val) {
    __asm__ __volatile__(
        "mov %0, %%cr3 \n\t"
        : : "r"(val)
    );
}

#endif /* _X86_H_ */
