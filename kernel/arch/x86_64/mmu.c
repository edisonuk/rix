#include <mmu.h>
#include <defines.h>
#include <string.h>
#include "kernel_aspace.h"

#include "../../mm/pmm.h"

/*
 * Largest linear and physical address size. Assume 48 bits linear and
 * 32 bits physical. Use cpuid 80000008h query to update later.
 */
unsigned char g_vaddr_width = 48; 
unsigned char g_paddr_width = 32;

/* initalized in start.S */
pt_entry_t pml4[NUM_PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
pt_entry_t pdp[NUM_PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
pt_entry_t pte[NUM_PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static bool supported_1gb_pages = false;

/*========================= Address Validity =========================*/

int mmu_check_vaddr(vaddr_t vaddr) {
    /* check if vaddr is page aligned */
    if (!IS_PAGE_ALIGNED(vaddr)) {
        return 0;   
    }

    /* higher half cannonical address space */
    unsigned long long higher_half_min = 0xffff800000000000;
    
    /* lower half cannonical address space */
    unsigned long long lower_half_max = 0x00007fffffffffff;

    /* check if vaddr is cannonical */
    return (vaddr > lower_half_max || vaddr < higher_half_min);
}

int mmu_check_paddr(paddr_t paddr) {
    /* check if paddr is page aligned */
    if (!IS_PAGE_ALIGNED(paddr)) {
        return 0;
    }

    return paddr <= (((uint64_t)1ull << g_paddr_width) - 1);
}

/*===================== Page frame number getters =====================*/

/* 1GiB pages */
static inline uint64_t get_pfn_from_pdpe(uint64_t pdpe) {
    uint64_t pfn;
    pfn = (pdpe & X86_1GB_PAGE_FRAME);
    return pfn;
}

/* 2MiB pages */
static inline uint64_t get_pfn_from_pde(uint64_t pde) {
    uint64_t pfn;
    pfn = (pde & X86_2MB_PAGE_FRAME);
    return pfn;
}

/* 4KiB pages */
static inline uint64_t get_pfn_from_pte(uint64_t pte) {
    uint64_t pfn;
    pfn = (pte & X86_4KB_PAGE_FRAME);
    return pfn;
}

/*===================== Table entry getter routines =====================*/

static inline uint64_t
get_pml4e_from_pml4_table(vaddr_t vaddr, addr_t pml4t_base_addr) {
    uint64_t *pml4_table_base_ptr = (uint64_t*)pml4t_base_addr;
    uint32_t pml4e_index = VADDR_TO_PML4_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(pml4_table_base_ptr[pml4e_index]);
}

static inline uint64_t
get_pdpe_from_pdp_table(vaddr_t vaddr, addr_t pdpt_base_addr) {
    uint64_t *pdp_table_base_ptr = (uint64_t*)pdpt_base_addr;
    uint32_t pdpe_index = VADDR_TO_PDP_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(pdp_table_base_ptr[pdpe_index]);
}

static inline uint64_t
get_pde_from_pd_table(vaddr_t vaddr, addr_t pdt_base_addr) {
    uint64_t *pd_table_base_ptr = (uint64_t*)pdt_base_addr;
    uint32_t pde_index = VADDR_TO_PD_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(pd_table_base_ptr[pde_index]);
}

static inline uint64_t
get_pte_from_ptable(vaddr_t vaddr, addr_t pt_base_addr) {
    uint64_t *ptable_base_ptr = (uint64_t*)pt_base_addr;
    uint32_t pte_index = VADDR_TO_PT_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(ptable_base_ptr[pte_index]);
}

/*======================= Address mapping routines =======================*/

/** Walk the page table structures. */
mmu_status_t mmu_get_mapping(vaddr_t vaddr, addr_t pml4_base_addr,
                    pt_entry_t *last_valid_entry, uint64_t *out_flags, uint32_t *out_level) {
    uint64_t pml4e, pdpe, pde, pte;
    pml4e = get_pml4e_from_pml4_table(vaddr, pml4_base_addr);

    /* TODO: assert(pml4_base_addr) */

    if  (!(last_valid_entry) || !(out_flags) || !(out_level)) {
        return ERR_INVALID_ARGS;
    }

    *out_level = PL_512G;
    *last_valid_entry = pml4_base_addr;
    *out_flags = 0;

    /* check if pml4 table entry present */
    if (!(pml4e & X86_PAGE_BIT_P)) {
        return ERR_ENTRY_NOT_PRESENT;
    }

    *out_level = PL_1G;
    pdpe = get_pdpe_from_pdp_table(vaddr, pml4e);

    if (!(pdpe & X86_PAGE_BIT_P)) {
        *last_valid_entry = pml4e;
        return ERR_ENTRY_NOT_PRESENT;
    }

    /* 1 GiB huge pages */
    if ((pdpe & X86_PAGE_BIT_PS) && supported_1gb_pages) {
        *last_valid_entry = get_pfn_from_pdpe(X86_VIRT_TO_PHYS(pdpe)) + ((uint64_t)vaddr & X86_1GB_PAGE_OFFSET_MASK);
        goto done;
    }

    *out_level = PL_2M;
    pde = get_pde_from_pd_table(vaddr, pdpe);

    if (!(pde & X86_PAGE_BIT_P)) {
        *last_valid_entry = pdpe;
        return ERR_ENTRY_NOT_PRESENT;
    }

    /* 2 MiB huge pages */
    if (pde & X86_PAGE_BIT_PS) {
        *last_valid_entry = get_pfn_from_pde(X86_VIRT_TO_PHYS(pde)) + ((uint64_t)vaddr & X86_2MB_PAGE_OFFSET_MASK);
        goto done;
    }

    /* 4 KiB pages */
    *out_level = PL_4K;
    pte = get_pte_from_ptable(vaddr, pde);

    if (!(pte & X86_PAGE_BIT_P)) {
        *last_valid_entry = pde;
        return ERR_ENTRY_NOT_PRESENT;
    }

    *last_valid_entry = get_pfn_from_pte(X86_VIRT_TO_PHYS(pte)) + ((uint64_t)vaddr & X86_4KB_PAGE_OFFSET_MASK);
    *out_flags = (pte & X86_PAGE_ENTRY_FLAGS_MASK);

done:
    *out_level = PL_FRAME;
    return NO_ERROR;
}

/*======================= Table entry update routines =======================*/

static void update_pt_entry(vaddr_t vaddr, uint64_t pde, paddr_t paddr, uint64_t flags) {
    uint64_t *pt_table_ptr = (uint64_t *)(pde & X86_4KB_PAGE_FRAME);
    uint32_t pt_index = (((uint64_t)vaddr >> PT_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
    
    /* set the address and present bit */
    pt_table_ptr[pt_index] = (uint64_t)paddr;
    pt_table_ptr[pt_index] |= X86_PAGE_BIT_P;

    if (flags & X86_PAGE_BIT_U)
        pt_table_ptr[pt_index] |= X86_PAGE_BIT_U;
}

static void update_pd_entry(vaddr_t vaddr, uint64_t pdpe, pt_entry_t map, uint64_t flags) {
    uint64_t *pd_table_ptr = (uint64_t *)(pdpe & X86_4KB_PAGE_FRAME);
    uint32_t pd_index = (((uint64_t)vaddr >> PD_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
    
    /* set the address and present bit */
    pd_table_ptr[pd_index] = (uint64_t)map;
    pd_table_ptr[pd_index] |= X86_PAGE_BIT_P | X86_PAGE_BIT_RW;

    if (flags & X86_PAGE_BIT_U)
        pd_table_ptr[pd_index] |= X86_PAGE_BIT_U;
}

static void update_pdp_entry(vaddr_t vaddr, uint64_t pml4e, pt_entry_t map, uint64_t flags) {
    uint64_t *pdp_table_ptr = (uint64_t *)(pml4e & X86_4KB_PAGE_FRAME);
    uint32_t pdp_index = (((uint64_t)vaddr >> PDP_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
    
    /* set the address and present bit */
    pdp_table_ptr[pdp_index] = (uint64_t)map;
    pdp_table_ptr[pdp_index] |= X86_PAGE_BIT_P | X86_PAGE_BIT_RW;

    if (flags & X86_PAGE_BIT_U)
        pdp_table_ptr[pdp_index] |= X86_PAGE_BIT_U;
}

static void update_pml4_entry(vaddr_t vaddr, uint64_t pml4_addr, pt_entry_t map, uint64_t flags) {
    uint64_t *pml4_table_ptr = (uint64_t *)(pml4_addr);
    uint32_t pml4_index = (((uint64_t)vaddr >> PML4_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
    
    /* set the address and present bit */
    pml4_table_ptr[pml4_index] = map;
    pml4_table_ptr[pml4_index] |= X86_PAGE_BIT_P | X86_PAGE_BIT_RW;

    if (flags & X86_PAGE_BIT_U)
        pml4_table_ptr[pml4_index] |= X86_PAGE_BIT_U;
}

/** Allocates a page table. */
static pt_entry_t *allocate_page_table(void) {
    paddr_t paddr;
    vm_page_t *page;

    pt_entry_t *page_ptr = pmm_alloc_kpage();

    if (page_ptr)
        memset(page_ptr, 0, PAGE_SIZE);

    return page_ptr;
}

mmu_status_t mmu_map_addr(vaddr_t vaddr, paddr_t paddr, addr_t pml4, uint64_t mmu_flags) {
    /* check if vaddr and paddr are cannonical and page aligned */
    if (!mmu_check_vaddr(vaddr) && !mmu_check_paddr(paddr)) {
        return ERR_INVALID_ARGS;
    }

    uint64_t pml4e, pdpe, pde, pte;
    uint32_t created_pdp = 0, created_pd = 0;

    mmu_status_t ret = NO_ERROR;

    pt_entry_t *m = NULL;   /* used to create tables */

    pml4e = get_pml4e_from_pml4_table(vaddr, pml4);
    
    /* check for present flag bit in the entry */
    if (!(pml4e & X86_PAGE_BIT_P)) {
        /* create a new pdp table */
        m = _map_alloc_page();
        if (m == NULL) {
            ret = ERR_OUT_OF_MEMORY;
            goto done;
        }

        update_pml4_entry(vaddr, pml4, X86_VIRT_TO_PHYS(m), );
        pml4e = (uint64_t)m;
        created_pdp = 1;
    }

    if (!created_pdp) {
        /* in case pdpe was not created retrieve the pdp entry */
        pdpe = get_pdpe_from_pdp_table(vaddr, pml4e);
    }

    /* create a pd table if the pdp table was just created 
       or the pdp entry does not exist */
    if (created_pdp || !(pdpe & X86_PAGE_BIT_P)) {
        /* Create a new pd table */
        m = _map_alloc_page();
        if (m == NULL) {
            ret = ERR_OUT_OF_MEMORY;

            if (created_pdp)
                goto clean_pdp;

            goto done;
        }

        /* TODO: maybe expand 1GiB Pages */

        update_pdp_entry(vaddr, pml4e, X86_VIRT_TO_PHYS(m));
        pdpe = (uint64_t)m;
        created_pd = 1;
    }

    if (!created_pd) {
        pde = get_pdpe_from_pdp_table(vaddr, pdpe);
    }

    if (created_pd || !(pde & X86_PAGE_BIT_P)) {
        /* Create a new page table */
        bool is_2mib_page = !!(pde & X86_PAGE_BIT_PS);

        m = _map_alloc_page();
        if (m == NULL) {
            ret = ERR_OUT_OF_MEMORY;

            if (created_pd)
                goto clean_pd;

            goto done;
        }

        /* if the page directory entry was pointing to a 2MiB page expand 
           it to 4KiB pages */
        if (is_2mib_page) {
            uint64_t pd_paddr = X86_VIRT_TO_PHYS(pde) & X86_4KB_PAGE_FRAME;
            uint64_t *pt_table = (uint64_t *)((uint64_t)m & X86_4KB_PAGE_FRAME);
            uint32_t index;

            /* fill up the 512 entires with 4KiB pages */
            for (index = 0; index < NUM_PT_ENTRIES; ++index) {
                pt_table[index] = (uint64_t) pd_paddr;
                pt_table[index] |= X86_PAGE_BIT_P;

                paddr += PAGE_SIZE;
            }
        }

        update_pd_entry(vaddr, pdpe, X86_VIRT_TO_PHYS(m));
        pde = (uint64_t)m;
    }

    /* finally update the page table with the physical address */
    update_pt_entry(vaddr, pde, paddr);

    /* update/flush TLB */

clean_pd:
    if (created_pd)
        pmm_free_page(paddr_to_page(X86_PHYS_TO_VIRT(pdpe)));

clean_pdp:
    if (created_pdp)
        pmm_free_page(paddr_to_page(X86_PHYS_TO_VIRT(pml4e)));

done:
    return ret;
}

/**
 * Check wall the entires of the page table for present bit.
 *
 * @returns True if none of the entries has present bit set.
 */
static inline bool is_page_table_clear(const pt_entry_t* page_table) {
    uint32_t lower_idx;
    for (lower_idx = 0; lower_idx < NUM_PT_ENTRIES; ++lower_idx) {
        if (page_table[lower_idx] & X86_PAGE_BIT_P) {
            return false;
        }
    }

    return true;
}

static void mmu_unmap_entry(vaddr_t vaddr, page_level_t level, vaddr_t table_entry) {
    uint32_t offset = 0, next_level_offset = 0;
    vaddr_t *table, *next_table_addr, value;

    next_table_addr = NULL;
    table = (vaddr_t *)(table_entry & X86_4KB_PAGE_FRAME);

    switch (level) {
    case PL_4K:
        offset = (((uint64_t)vaddr >> PT_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
        next_table_addr = (vaddr_t *)X86_PHYS_TO_VIRT(table[offset]);
    
        /* if not present return */
        if (!((X86_PHYS_TO_VIRT(table[offset]) & X86_PAGE_BIT_P)))
            return;
    
        break;

    case PL_2M:
        offset = (((uint64_t)vaddr >> PD_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
        next_table_addr = (vaddr_t *)X86_PHYS_TO_VIRT(table[offset]);
    
        /* if not present return */
        if (!((X86_PHYS_TO_VIRT(table[offset]) & X86_PAGE_BIT_P)))
            return;
    
        break;

    case PL_1G:
        offset = (((uint64_t)vaddr >> PDP_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
        next_table_addr = (vaddr_t *)X86_PHYS_TO_VIRT(table[offset]);
    
        /* if not present return */
        if (!((X86_PHYS_TO_VIRT(table[offset]) & X86_PAGE_BIT_P)))
            return;
    
        break;

    case PL_512G:
        offset = (((uint64_t)vaddr >> PML4_SHIFT) & ((1UL << ADDR_OFFSET) - 1));
        next_table_addr = (vaddr_t *)X86_PHYS_TO_VIRT(table[offset]);
    
        /* if not present return */
        if (!((X86_PHYS_TO_VIRT(table[offset]) & X86_PAGE_BIT_P)))
            return;
    
        break;

    case PL_FRAME:
    default:
        return;
    }

    mmu_unmap_entry(vaddr, level - 1, (vaddr_t) next_table_addr);

    next_table_addr = (vaddr_t *)((vaddr)(next_table_addr) & X86_4KB_PAGE_FRAME);
    if (level > PL_4K) {
        if (!is_page_table_clear(next_table_addr)) {
            return;
        }

        pmm_free_page(paddr_to_page(X86_VIRT_TO_PHYS(next_table_addr)));
    }

    if (X86_PHYS_TO_VIRT(table[offset]) & X86_PAGE_BIT_P) {
        /* TODO: disable interrupts */
        value = table[offset];
        value = value;
        table[offset] = value;
        /* TODO: enable interrupts */
    }
}

mmu_status_t mmu_unmap_addr(vaddr_t vaddr, paddr_t paddr, addr_t pml4_base_addr) {
    
}

int mmu_init(void) {
    /* query the address sizes */
    uint32_t addr_width = cpuid_get_addr_width();

    /* Bits 07-00: Physical Address bits */
    /* Bits 15-08: Linear Address bits */    
    uint8_t vaddr_width = (uint8_t)(addr_width & 0xff);
    uint8_t paddr_width = (uint8_t)((addr_width >> 8) & 0xff);

    if (g_vaddr_width < vaddr_width) {
        g_vaddr_width = vaddr_width;
    }

    if (g_paddr_width < paddr_width) {
        g_paddr_width = paddr_width;
    }

    /* flush tlb */
    set_cr3(get_cr3());
}
