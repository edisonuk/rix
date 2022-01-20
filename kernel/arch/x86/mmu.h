#ifndef _X86_MMU_H_
#define _X86_MMU_H_

#include <x86.h>

#define X86_PAGE_BIT_PRESENT    0x0000 /* Present               */
#define X86_PAGE_BIT_RW         0x0001 /* Writable              */
#define X86_PAGE_BIT_PWT        0x0001 /* Page Write Through    */
#define X86_PAGE_BIT_RCD        0x0001 /* Page Cache Disabled   */
#define X86_PAGE_BIT_PS         0x0007 /* (2MB Pages)           */
#define X86_PAGE_BIT_GLOBAL     0x0008 /* Global                */

#define X86_MMU_PG_FLAGS (X86_MMU_PG_P | X86_MMU_PG_RW)

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
#define NO_OF_PT_ENTRIES    512

/* page frame number mask */
#define X86_PAGE_FRAME      (0x000ffffffffff000ul)
#define X86_2MB_PAGE_FRAME  (0x000fffffffe00000ul)
#define X86_1GB_PAGE_FRAME  (0x000fffffc0000000ul)

/* page offset mask */
#define X86_PAGE_OFFSET_MASK        ((1ul << PT_SHIFT) - 1)
#define X86_2MB_PAGE_OFFSET_MASK    ((1ul << PD_SHIFT) - 1)
#define X86_1GB_PAGE_OFFSET_MASK    ((1ul << PDP_SHIFT) - 1)

/* extrating incides from vaddr */
#define VADDR_TO_PML4_INDEX(vaddr)  ((vaddr) >> PDP_SHIFT) & ((1ul << ADDR_OFFSET) - 1)
#define VADDR_TO_PDP_INDEX(vaddr)   ((vaddr) >> PDP_SHIFT) & ((1ul << ADDR_OFFSET) - 1)
#define VADDR_TO_PD_INDEX(vaddr)    ((vaddr) >> PD_SHIFT) & ((1ul << ADDR_OFFSET) - 1)
#define VADDR_TO_PT_INDEX(vaddr)    ((vaddr) >> PT_SHIFT) & ((1ul << ADDR_OFFSET) - 1)

/* !(pointer % alignment) */
#define IS_ALIGNED(_pointer, _align)    \
    (!(((uintptr_t)(_pointer)) & (((uintptr_t)(_align)) - 1))) 

#define IS_PAGE_ALIGNED(_pointer) IS_ALIGNED(_pointer, PAGE_SIZE)

/* 8 bytes page table entry */
typedef unsigned long long pt_entry_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;
typedef uintptr_t addr_t;

/* 4 level paging */
enum page_level {
    PL_NONE,
    PL_4K,      /* PT   */
    PL_2M,      /* PD   */
    PL_1G,      /* PDP  */
    PL_512G,    /* PML4 */
    PL_NUM
};

/* check if virtual address is page aligned and cannonical */
int mmu_check_vaddr(vaddr_t vaddr);
int mmu_check_paddr(paddr_t paddr);

/**
 * Walks the page table. Returns page frame number as last_valid_entry
 * if a valid page was found else returns the previous valid level page table
 * entry.
 */
int mmu_lookup_mapping(vaddr_t vaddr, addr_t pml4_base_addr,
                    unsigned long *last_valid_entry, uint32_t *level);
int mmu_map_addr(vaddr_t vaddr, paddr_t paddr);

void mmu_init(void);

#endif /* _X86_MMU_H_ */
