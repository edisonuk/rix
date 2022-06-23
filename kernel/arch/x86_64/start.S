#include "../../asm.h"
#include "reg_defs.h"
#include "mmu.h"

#include "check_long_mode.S"

/* Physical address */
#define PHY_ADDR_DELTA  0
#define PHY_ADDR(x)     (x - PHY_ADDR_DELTA)

.section .text.boot, "ax", @progbits
BEGIN_FUNCTION(_start)
    mov $(kstack_bottom), %esp

    /* check for long mode support */
    call check_long_mode
    testb %eax, %eax
    jz .Lno_long_mode

    /* x64 paging (CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1) */
    
    mov %cr4, %eax
    or $(CR4_PAE_BIT), %eax
    mov %eax, %cr4 

    mov $(IA32_MSR_EFER), %ecx
    rdmsr
    or $(IA32_MSR_EFER_LME), %eax
    wrmsr

    mov %cr0, %eax
    or $(CR0_PE_BIT | CR0_PG_BIT), %eax
    mov %eax, %cr0

.Lzero_bss:
    /* zero the bss section */
    xor %eax, %eax
    mov $PHY_ADDR(__bss_start), %edi
    mov $PHY_ADDR(__bss_end), %ecx
    sub %edi, %ecx
    rep stosb

    movl $PHY_ADDR(pml4), %eax
    movl %eax, %cr3

.Lsetup_paging64:
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

.Lno_long_mode:
    /* no long mode support */
    
END_FUNCTION(_start)

.section .bss
.align 16
DATA(kstack_bottom)
    .skip 8192
DATA(kstack_top)