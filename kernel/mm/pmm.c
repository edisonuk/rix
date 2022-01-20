#include <pmm.h>

static struct list_node arena_list = LIST_INITIAL_VALUE(arena_list);
static LIST_NODE(free_list);

/* whether or not a page belong to the arena page array */
#define PAGE_BELONGS_TO_ARENA_PAGE_ARRAY(page, arena)                                           \
    ((uintptr_t)(page) >= (uintptr_t)(arena)->page_array) &&                                    \
    ((uintptr_t)(page) < ((uintptr_t)(arena)->page_array + (arena)->size / PAGE_SIZE))

#define ARENA_PAGE_COUNT(arena) ((arena)->size / PAGE_SIZE)

#define IS_FREE_PAGE()

int pmm_add_arena(pmm_arena_t *arena) {
    
}

int pmm_alloc_page(vm_page_t *page_out) {
    vm_page_t* page = list_remove_head_type(&free_list, vm_page, queue_node);
    if (!page) {
        /* no memory */
        return 0;
    }

    return 1;
}

int pmm_alloc_pages(uint32_t count, struct list_node* list) {
    if (count == 0) {
        return 1;
    } else if (count == 1) {
        vm_page* page;
        int status = pmm_alloc_page(&page);
        if (status == 1) {
            /* add allocated pages to the list */
            list_add_tail(list, &page->queue_node);
        }

        return status;
    }

    /* num pages allocated */
    uint32_t num_pages_allocated = 0;

    /* mutex lock */

    /* remove pages from free list */
    pmm_arena_t *pa;
    list_for_every_entry() {
        while (num_pages_allocated < count) {
            num_pages_allocated++;
        }
    }

    /* release lock */

    return num_pages_allocated;
}

int pmm_free_pages(struct list_node* list) {
}
