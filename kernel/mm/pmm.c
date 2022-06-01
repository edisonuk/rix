#include "pmm.h"
#include "mm_p.h"

#include <stdbool.h>
#include <string.h>

#include "../list.h"
#include "../mutex.h"

/** Linked list of arenas. */
static LIST_HEAD(arena_list);

#define PAGE_BELONGS_TO_ARENA(page, arena)                                              \
    ((uintptr_t)(page) >= (uintptr_t)(arena)->page_array) &&                            \
    ((uintptr_t)(page) < ((uintptr_t)(arena)->page_array + ARENA_PAGE_COUNT(arena)))

#define PAGE_INDEX_IN_ARENA(page, arena)    \
    (((uintptr_t)page - (uintptr_t)(arena)->page_array) / sizeof(vm_page_t))

#define PAGE_ADDRESS_FROM_ARENA(page, arena)    \
    (paddr_t)PAGE_INDEX_IN_ARENA(page, arena) * PAGE_SIZE + (arena)->base;

#define ARENA_PAGE_COUNT(arena) (arena->size / PAGE_SIZE)

#define ADDRESS_BELONGS_TO_ARENA(address, arena)    \
    ((address) >= arena->base && (address) <= arena->base + arena->size - 1)


static inline bool page_is_free(const vm_page_t *page) {
    return !(page->flags & VM_PAGE_FLAG_NONFREE);
}

void *paddr_to_kvaddr(paddr_t pa) {

}

paddr_t vaddr_to_paddr(void *va) {

}

paddr_t page_to_paddr(const vm_page_t *page) {
    pmm_arena_t *arena;
    list_for_each_entry(arena, &arena_list, node) {
        if (PAGE_BELONGS_TO_ARENA(page, arena)) {
            return PAGE_ADDRESS_FROM_ARENA(page, arena);
        }
    }
    return -1;
}

vm_page_t *paddr_to_page(paddr_t addr) {
    pmm_arena_t *arena;
    list_for_each_entry(arena, &arena_list, node) {
        if (ADDRESS_BELONGS_TO_ARENA(addr, arena)) {
            size_t index = (addr - arena->base) / PAGE_SIZE;
            return &arena->page_array[index];
        }
    }
    return NULL;
}

/*============================ Page Arena Routines ============================*/

pmm_status_t pmm_add_arena(pmm_arena_t *arena) {
    /* TODO: assert(IS_PAGE_ALIGNED(arena->base)) */
    /* TODO: assert(IS_PAGE_ALIGNED(arena->size)) */
    /* TODO: assert(arena->size > 0) */

    if (!(arena->size > 0)) {
        return ERR_INVALID_ARENA_SIZE;
    }

    /* walk the arena list and add arena based on priority order */
    pmm_arena_t *a;
    list_for_each_entry(a, &arena_list, node) {
        /* add before the one with lower priority */
        if (a->priority > arena->priority) {
            /* add the new area before this arena */
            list_add_tail(&a->node, &arena->node);
            goto done_add;
        }
    }

    /* walked off the end, add it to the end of the list */
    list_add_tail(&arena_list, &arena->node);

done_add:
    arena->free_count = 0;
    list_initialize(&arena->free_list);

    /* allocate an array of pages */
    size_t page_count = ARENA_PAGE_COUNT(arena);
    arena->page_array = balloc(page_count * sizeof(vm_page_t));

    /* zero all the pages */
    memset(&arena->page_array, 0, page_count * sizeof(vm_page_t));

    /* add the allocated pages to free list */
    for (int i = 0; i < page_count; ++i) {
        vm_page_t* page = &arena->page_array[i];

        list_add_tail(&arena->free_list, &page->node);
        arena->free_count++;
    }
}

int pmm_alloc_pages(uint32_t count, list_node_t *list) {
    /* fast path */
    if (count == 0) {
        return 1;
    } else if (count == 1) {
        vm_page_t* page;
        int status = pmm_alloc_page(page);
        if (status == 1) {
            /* add allocated pages to the list */
            list_add_tail(list, &page->node);
        }

        return status;
    }

    /* num pages allocated */
    uint32_t num_pages_allocated = 0;

    /* mutex lock */

    /* remove pages from free list */
    pmm_arena_t *arena;
    list_for_each_entry(arena, &arena_list, node) {
        while (num_pages_allocated < count && arena->free_count > 0) {
            vm_page_t *page = list_remove_head_type(&arena->free_list, vm_page_t, node);
            if (!page)
                goto done;

            arena->free_count--;

            page->flags |= VM_PAGE_FLAG_NONFREE;
            list_add_tail(list, &page->node);

            num_pages_allocated++;
        }

        /* break when we have already allocated to required number of pages */
        if (num_pages_allocated == count)
            break;
    }

done:
    /* release lock */
    return num_pages_allocated;
}

int pmm_alloc_page(vm_page_t *page_out) {
    /* mutex lock */

    /* walk through the arena searching for free page */
    pmm_arena_t *arena;
    list_for_each_entry(arena, &arena_list, node) {
        /* allocate a page if the arena has free page */
        if (arena->free_count > 0) {
            vm_page_t *page = list_remove_head_type(&arena->free_list, vm_page_t, node);
            if (!page)
                goto done;

            arena->free_count--;

            page->flags |= VM_PAGE_FLAG_NONFREE;
            page_out = page;
        }
    }

done:
    /* mutex release */
    return 1;
}

int pmm_alloc_range(paddr_t address, size_t count, list_node_t* list) {
    /* make sure the address is page aligned */
    address = ROUNDDOWN(address, PAGE_SIZE);

    size_t num_pages_allocated = 0;
    if (count == 0)
        return 0;

    /* mutex lock */

    /* walk through the arena, see if the physical page belongs to it */
    pmm_arena_t *arena;
    list_for_each_entry(arena, &arena_list, node) {
        while (num_pages_allocated < count && ADDRESS_BELONGS_TO_ARENA(address, arena)) {
            size_t index = (address - arena->base) / PAGE_SIZE;

            /* TODO: DEBUG_ASSERT(index < a->size / PAGE_SIZE); */

            vm_page_t *page = &arena->page_array[index];
            if (page->flags & VM_PAGE_FLAG_NONFREE) {
                /* page is already allocated */
                break;
            }

            list_delete(&page->node);
            page->flags |= VM_PAGE_FLAG_NONFREE;
            list_add_tail(list, &page->node);

            arena->free_count--;
            num_pages_allocated++;

            address += PAGE_SIZE;
        }

        if (num_pages_allocated == count)
            break;
    }

done:
    /* mutex release */
    return num_pages_allocated;
}

size_t pmm_free(list_node_t *head) {
    /* mutex lock */

    size_t count = 0;
    while(!list_is_empty(head)) {
        vm_page_t *page = list_remove_head_type(head, vm_page_t, node);

        /* find the arena this page belongs to and add the page to its free-list */
        pmm_arena_t *arena;
        list_for_each_entry(arena, &arena_list, node) {
            if (PAGE_BELONGS_TO_ARENA(page, arena)) {
                page->flags &= ~VM_PAGE_FLAG_NONFREE;
                
                list_add(&arena->free_list, &page->node);
                arena->free_count++;
                count++;
                break;
            }
        }
    }

    /* release mutex */
    return count;
}

size_t pmm_free_page(vm_page_t *page) {
    list_node_t list;
    list_initalize(list);

    list_add(&list, &page->node);

    return pmm_free(&list);
}

pmm_status_t
pmm_alloc_contiguous(size_t count, uint8_t align_log2, size_t* out_count,
                    paddr_t *pa_out, list_node_t* list) {
    if (count == 0)
        return 0;

    /* must be atleast 4KiB */
    if (align_log2 < PAGE_SIZE_SHIFT)
        align_log2 = PAGE_SIZE_SHIFT;

    /* mutex lock */

    pmm_arena_t *arena;
    list_for_each_entry(arena, &arena_list, node) {
        if (arena->flags) {
            paddr_t rounded_base = ROUNDUP(arena->base, 1UL << align_log2);
            if (rounded_base < arena->base || rounded_base > arena->base + arena->size - 1)
                continue;

            uint32_t aligned_offset = (rounded_base - arena->base) / PAGE_SIZE;
            uint32_t start_idx = aligned_offset;

retry:
            /* keep searching while we are still within the arena (size + count < end of arena) */
            while((start_idx < ARENA_PAGE_COUNT(arena)) && (start_idx + count) <= ARENA_PAGE_COUNT(arena)) {
                vm_page_t *page = &arena->page_array[start_idx];
                for (uint32_t i = 0; i < count; ++i) {
                    if (page->flags & VM_PAGE_FLAG_NONFREE) {
                        /* failed before reaching count, break and retry */
                        start_idx = ROUNDUP(start_idx - aligned_offset + i + 1, 1UL << (align_log2 - PAGE_SIZE_SHIFT)) + aligned_offset;
                        goto retry;
                    }
                    page++;
                }

                /* remove the pages out of the free-list */
                for (int i = start_idx; i < start_idx + count; ++i) {
                    page = &arena->page_array[i];

                    list_delete(&page->node);
                    page->flags |= VM_PAGE_FLAG_NONFREE;
                    arena->free_count--;

                    if (list)
                        list_add_tail(list, &page->node);
                }

                if (pa_out)
                    *pa_out = arena->base + start_idx * PAGE_SIZE;

                /* mutex release */
                *out_count = count;
                return NO_ERROR;
            }
        }
    }

    /* mutex release */
    return ERR_CONTIGUOUS_PAGES_NOT_FOUND;
}

void *pmm_alloc_kpages(int count, list_node_t *list) {
    if (count == 1) {
        vm_page_t* page;
        int status = pmm_alloc_page(page);

        if (!page) {
            return NULL;
        }

        return paddr_to_kvaddr(vm_page_to_vaddr())
    }
}

size_t pmm_free_kpages(void *ptr, uint32_t count) {

}