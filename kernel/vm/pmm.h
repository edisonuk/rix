#ifndef _PMM_H_
#define _PMM_H_

#include "../list.h"
#include "../types.h"
#include "../arch.h"
#include "../stdlib.h"
#include <stdint.h>

/** Per page structure */
typedef struct vm_page {
    uint32_t    flags;
    list_node_t node;
} vm_page_t;

#define VM_PAGE_FLAG_NONFREE (0x1)

typedef enum pmm_status {
    NO_ERROR,
    ERR_INVALID_ARENA_SIZE,
    ERR_CONTIGUOUS_PAGES_NOT_FOUND,
    ERR_INVLID_ARGS,
} pmm_status_t;

/* ------------------------------------------------------------------------
 *  Page Arena
 * ------------------------------------------------------------------------
 */

/**
 * @brief   Holds a fixed-sized array of pages. Pages are allocated during
 *          addition to arena list. Once the pages are freed they are
 *          added to the free_list.
 */
typedef struct pmm_arena {
    uint32_t    flags;
    uint32_t    priority;

    paddr_t     base;       /* Base address from where allocated
                             * pages starts.
                             */
    size_t      size;       /* Total size of the arena
                             * (page_count * PAGE_SIZE)
                             */
    
    size_t      free_count; /* Count of free pages. */

    list_node_t node;       /* Arena list */
    
    vm_page_t   *page_array;/* Array of pages allocated by this arena. */
    list_node_t free_list;  
} pmm_arena_t;

/**
 * @brief   Adds an arena to the arena list.
 *          Note: Size of the arena must not be 0.
 */
pmm_status_t    pmm_add_arena(pmm_arena_t *arena);

/* ------------------------------------------------------------------------
 *  Allocator routines
 * ------------------------------------------------------------------------
 */

/** @brief  Allocates count non-contiguous pages of physical memory. */
int             pmm_alloc_pages(uint32_t count, list_node_t* list);
int             pmm_alloc_page(vm_page_t *page_out);

/** @brief  Start allocating pages from the given address. */
int             pmm_alloc_range(paddr_t address, size_t count, list_node_t* list);

/**
 * @brief   Allocate a run of contiguous pages, aligned on log2 byte boundary (0-31)
 * 
 * @param pa_out If the optional physical address pointer is passed,
 *               return the address.
 * @param list  If the optional list is passed, append the allocate
 *              page structures to the tail of the list.
 */
pmm_status_t    pmm_alloc_contiguous(size_t count, uint8_t align_log2,
                    size_t* out_count, paddr_t *pa_out, list_node_t* list);

/**
 * @brief   Frees pages in the given list starting from head.
 * @returns Count of pages freed.
 */
size_t          pmm_free(list_node_t* head);
size_t          pmm_free_page(vm_page_t* page);

/**
 * @brief   Allocate pages from the kernel area and return the pointer
 *          in kernel space.
 */
void *          pmm_alloc_kpages(int count, list_node_t *list);
void *          pmm_alloc_kpage(void);

size_t          pmm_free_kpages(void *ptr, uint32_t count);

/** @brief  Physical address to virtual address */
void *          paddr_to_kvaddr(paddr_t pa);

/** @brief  Virtual address to physical address */
paddr_t         vaddr_to_paddr(void *va);

paddr_t         page_to_paddr(const vm_page_t *page);
vm_page_t *     paddr_to_page(paddr_t addr);

#endif /* _PMM_H_ */
