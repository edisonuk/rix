#ifndef _X86_MMU_H_
#define _X86_MMU_H_

#include "x86.h"
#include "../../types.h"

#define X86_PAGE_BIT_P          0x0001 /* Present                   */
#define X86_PAGE_BIT_RW         0x0002 /* Writable                  */
#define X86_PAGE_BIT_U          0x0004 /* User/Supervisor (0=s,1=u) */
#define X86_PAGE_BIT_PWT        0x0008 /* Page Write Through        */
#define X86_PAGE_BIT_PCD        0x0010 /* Page Cache Disabled       */
#define X86_PAGE_BIT_PS         0x0080 /* Page Size                 */
#define X86_PAGE_BIT_G          0x0100 /* Global                    */

#define X86_MMU_PG_FLAGS (X86_PAGE_BIT_P | X86_PAGE_BIT_RW)

/*
 * Linear Address mapping table indices
 *
 * 4KiB pages 
 *
 * 63-48    47-39    38-30    29-21    20-12    11 - 0
 * -------------------------------------------------------
 * |        |        |        |        |        |        |
 * -------------------------------------------------------
 * Sign     PML4     PDPT     PDT      PT       Page
 * Extend   index    index    index    index    offset
 *
 * 2MiB pages
 *
 * 63-48    47-39    38-30    29-21    20 - 0
 * -------------------------------------------------------
 * |        |        |        |        |                 |
 * -------------------------------------------------------
 * Sign     PML4     PDPT     PDT      Page
 * Extend   index    index    index    offset
 *
 * 1GiB pages
 *
 * 63-48    47-39    38-30    29 - 0
 * -------------------------------------------------------
 * |        |        |        |                          |
 * -------------------------------------------------------
 * Sign     PML4     PDPT     Page
 * Extend   index    index    offset
 */
#define PML4_SHIFT      39
#define PDP_SHIFT       30
#define PD_SHIFT        21
#define PT_SHIFT        12
#define ADDR_OFFSET     9

#define PDPT_ADDR_OFFSET    2

/* 512 page table entries */
#define NUM_PT_ENTRIES      512

/* 11:0 page table entry flags bits */
#define X86_PAGE_ENTRY_FLAGS_MASK   (0x0000000000000ffful)

/* dev manual: vol 3: section 4.5.4 */
#define X86_4KB_PAGE_FRAME          (0x000ffffffffff000ul)
#define X86_2MB_PAGE_FRAME          (0x000fffffffe00000ul)
#define X86_1GB_PAGE_FRAME          (0x000fffffc0000000ul)

/* page offset mask */
#define X86_4KB_PAGE_OFFSET_MASK    ((1ul << PT_SHIFT) - 1)
#define X86_2MB_PAGE_OFFSET_MASK    ((1ul << PD_SHIFT) - 1)
#define X86_1GB_PAGE_OFFSET_MASK    ((1ul << PDP_SHIFT) - 1)

/* extrating incides from vaddr */
#define VADDR_TO_PML4_INDEX(vaddr)  ((vaddr) >> PML4_SHIFT) & ((1ul << ADDR_OFFSET) - 1)
#define VADDR_TO_PDP_INDEX(vaddr)   ((vaddr) >> PDP_SHIFT) & ((1ul << ADDR_OFFSET) - 1)
#define VADDR_TO_PD_INDEX(vaddr)    ((vaddr) >> PD_SHIFT) & ((1ul << ADDR_OFFSET) - 1)
#define VADDR_TO_PT_INDEX(vaddr)    ((vaddr) >> PT_SHIFT) & ((1ul << ADDR_OFFSET) - 1)

/* physical memory is mapped at the base of the kernel address space */
#define X86_PHYS_TO_VIRT(x) ((uintptr_t)(x) + KERNEL_ASPACE_BASE)
#define X86_VIRT_TO_PHYS(x) ((uintptr_t)(x) - KERNEL_ASPACE_BASE)

/* !(pointer % alignment) */
#define IS_ALIGNED(_addr, _align)    \
    (!(((uintptr_t)(_addr)) & (((uintptr_t)(_align)) - 1))) 

#define IS_PAGE_ALIGNED(_addr) IS_ALIGNED(_addr, PAGE_SIZE)

/* 8 bytes page table entry */
typedef uint64_t pt_entry_t;

/* 4 level paging */
typedef enum page_level {
    PL_FRAME,   /* Page frame   */
    PL_4K,      /* PT           */
    PL_2M,      /* PD           */
    PL_1G,      /* PDP          */
    PL_512G,    /* PML4         */
    PL_NUM
} page_level_t;

typedef enum mmu_status {
    NO_ERROR,
    ERR_ENTRY_NOT_PRESENT,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_ARGS,
} mmu_status_t;

/*============================ MMU Routines ============================*/

/* check if virtual address is page aligned and cannonical */
int mmu_check_vaddr(vaddr_t vaddr);
int mmu_check_paddr(paddr_t paddr);

/**
 * Walks the page table. Returns page frame number as last_valid_entry
 * if a valid page was found else returns the previous valid level page table
 * entry.
 */
mmu_status_t mmu_get_mapping(vaddr_t vaddr, addr_t pml4_base_addr,
                    pt_entry_t *last_valid_entry, uint64_t *out_flags, uint32_t *out_level);

/**
 * Walk the page table structures to see if the mapping between a virtual address
 * and a physical exists. Also check the flags.
 */
mmu_status_t mmu_check_mapping(vaddr_t vaddr, paddr_t paddr, addr_t pml4_base_addr);

mmu_status_t mmu_map_addr(vaddr_t vaddr, paddr_t paddr, addr_t pml4_base_addr, uint64_t mmu_flags);
mmu_status_t mmu_unmap_addr(vaddr_t vaddr, paddr_t paddr, addr_t pml4_base_addr);

int mmu_init(void);

#endif /* _X86_MMU_H_ */
