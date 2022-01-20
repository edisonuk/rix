#ifndef _VM_H_
#define _VM_H_

#include <list.h>

/* Physical Memory Manager */

typedef struct vm_page {
    struct list_node queue_node;
} vm_page_t;

#define VM_PAGE_FLAG_NONFREE (0x1)

typedef struct pmm_arena {
    struct vm_page* page_array;
} pmm_arena_t;

int pmm_add_arena(pmm_arena_t *arena);

/* allocator routines */
int pmm_alloc_pages(uint32_t count, list_node_t* list);
int pmm_alloc_page(vm_page_t** page, paddr_t* pa);

int pmm_free_pages(list_node_t* list);

#endif /* _VM_H_ */
