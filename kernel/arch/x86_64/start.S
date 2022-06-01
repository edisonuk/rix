#include <asm.h>
#include <reg_defs.h>
#include <mmu.h>

#include <check_long_mode.S>

/* Physical address */
#define PHYS_ADDR_DELTA 
#define PHY_ADDR(x) (x - PHY_ADDR_DELTA)

.section .text.boot, "ax", @progbits
BEGIN_FUNCTION(_start)
    /* set stack pointer register to initial kernel stack */
    mov $(kbootstrap_stack_bottom), %esp

    /* check for long mode support */
    /* call check_long_mode
    testb %eax, %eax
    jz .Lno_long_mode */

    /* x64 paging (CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1) */
    
    /* enable PAE */
    mov %cr4, %eax
    or $(CR4_PAE_BIT), %eax
    mov %eax, %cr4 

    /* enable long mode by setting the LME flag */
    mov $(IA32_MSR_EFER), %ecx
    rdmsr
    or $(IA32_MSR_EFER_LME), %eax
    wrmsr

    /* enable paging and protected mode */
    mov %cr0, %eax
    or $(CR0_PE_BIT | CR0_PG_BIT), %eax
    mov %eax, %cr0

    /* zero the bss section */
    xor %eax, %eax
    mov $PHY_ADDR(__bss_start), %edi
    mov $PHY_ADDR(__bss_end), %ecx
    sub %edi, %ecx  /* calculate size of bss section */
    rep stosb

    /* load CR3 with physical address of pml4 */
    movl $PHY_ADDR(pml4), %eax
    movl %eax, %cr3

.Lsetup_paging64:
    /* 4 level paging */
    /* build PML4 */

    mov $PHY_ADDR(pdp), %eax
    or $(X86_MMU_PG_FLAGS), %eax
    mov %eax, PHY_ADDR(pdp)

    mov $PHY_ADDR(pde), %eax
    or $(X86_MMU_PG_FLAGS), %eax
    mov %eax, PHY_ADDR(pde)

    /* page table */
    mov $PHY_ADDR(pte), %eax
    or $(X86_MMU_PG_FLAGS), %eax
    mov %eax, PHY_ADDR(pte)
    
END_FUNCTION(_start)

.section .bss
.align 16
DATA(kstack_bottom)
    .skip 8192 /* 8KiB kernel stack */
DATA(kstack_top)