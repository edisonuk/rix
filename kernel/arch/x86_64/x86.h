#ifndef _X86_H_
#define _X86_H_

#include <reg_defs.h>
#include <stdbool.h>
#include "../../types.h"

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ __volatile__(
        "cpuid\n\t"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0)
    );
}

static inline void cpuid_c(uint32_t leaf, uint32_t csel, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ __volatile__(
        "cpuid\n\t"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(csel)
    );
}

static unsigned long cpuid_get_addr_width(void) {
    uint32_t eax, ebx, ecx, edx;
 
    cpuid(0x80000008, &eax, &ebx, &ecx, &edx);

    /* Bits 07-00: Physical Address bits */
    /* Bits 15-08: Linear Address bits */
    return (eax & 0xff);
}

static inline unsigned long get_cr0(void) {
    unsigned long rv;

    __asm__ __volatile__(
        "mov %%cr0, %0 \n\t"
        : "=r"(rv)
    );

    return rv;
}

static inline void set_cr0(unsigned long val) {
    __asm__ __volatile__(
        "mov %0, %%cr0 \n\t"
        : : "r"(val)
    );
}

static inline unsigned long get_cr3(void) {
    unsigned long rv;

    __asm__ __volatile__(
        "mov %%cr3, %0 \n\t"
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

static inline unsigned long get_cr4(void) {
    unsigned long rv;

    __asm__ __volatile__(
        "mov %%cr4, %0 \n\t"
        : "=r"(rv)
    );

    return rv;
}

static inline void set_cr4(unsigned long val) {
    __asm__ __volatile__(
        "mov %0, %%cr4 \n\t"
        : : "r"(val)
    );
}

static inline bool is_paging_enabled(void) {
    if (get_cr0() & CR0_PG_BIT)
        return true;

    return false;
}

static inline bool is_PAE_enabled(void) {
    if (is_paging_enabled() == false)
        return false;

    if (get_cr4() & CR4_PAE_BIT)
        return true;

    return false;
}

typedef unsigned long x86_flags_t;

static inline x86_flags_t save_flags(void) {
    x86_flags_t state;

    __asm__ __volatile__(
        "pushf;"
        "pop %0"
        : "=rm" (state)
        :: "memory"
    );

    return state;
}

static inline void restore_flags(x86_flags_t flags) {
    __asm__ __volatile__(
        "push %0;"
        "popf"
        :: "g" (flags)
        : "memory", "cc"
    );
}

#endif /* _X86_H_ */
