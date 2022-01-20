#include <mmu.h>
#include <defines.h>

/*
 * Largest linear and physical address size.
 * Assume 48 bits linear and 32 bits physical. 
 * Use CPUID EAX=80000008h query to update later.
 */
unsigned char g_vaddr_size = 48; 
unsigned char g_paddr_size = 32;

#define KERNEL_ASPACE_BASE 0xffff800000000000   /* -512GiB */  

/* initalized in start.S */
pt_entry_t pml4[NO_OF_PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
pt_entry_t pdp[NO_OF_PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
pt_entry_t pte[NO_OF_PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

int supported_1gb_pages;

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

/* 4KiB pages */
static inline uint64_t get_pfn_from_pte(uint64_t pte) {
    uint64_t pfn;
    pfn = (pte & X86_PG_FRAME);
    return pfn;
}

/* 2MiB pages */
static inline uint64_t get_pfn_from_pde(uint64_t pde) {
    uint64_t pfn;
    pfn = (pde & X86_2MB_PG_FRAME);
    return pfn;
}

/* 1GiB pages */
static inline uint64_t get_pfn_from_pdpe(uint64_t pdpe) {
    uint64_t pfn;
    pfn = (pdpe & X86_1GB_PG_FRAME);
    return pfn;
}

static inline uint64_t
get_pml4e_from_pml4_table(vaddr_t vaddr, addr_t pml4t_base_addr) {
    uint32_t pml4e_index;
    uint64_t *pml4_table_base_ptr = (uint64_t*)pml4_table_addr;
    pml4e_index = VADDR_TO_PML4_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(pml4e_table_base_ptr[pml4_index]);
}

static inline uint64_t
get_pdpe_from_pdp_table(vaddr_t vaddr, addr_t pdpt_base_addr) {
    uint32_t pdpe_index;
    uint64_t *pdp_table_base_ptr = (uint64_t*)pdpt_base_addr;
    pdpe_index = VADDR_TO_PDP_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(pdp_table_base_ptr[pdpe_index]);
}

static inline uint64_t
get_pde_from_pd_table(vaddr_t vaddr, addr_t pdt_base_addr) {
    uint32_t pde_index;
    uint64_t *pd_table_base_ptr = (uint64_t*)pdt_base_addr;
    pde_index = VADDR_TO_PD_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(pd_table_base_ptr[pde_index]);
}

static inline uint64_t
get_pde_from_ptable(vaddr_t vaddr, addr_t pt_base_addr) {
    uint32_t pte_index;
    uint64_t *ptable_base_ptr = (uint64_t*)pt_base_addr;
    pte_index = VADDR_TO_PT_INDEX(vaddr);
    
    return X86_PHYS_TO_VIRT(ptable_base_ptr[pte_index]);
}

int mmu_lookup_mapping(vaddr_t vaddr, addr_t pml4_base_addr,
                    unsigned long *last_valid_entry, uint32_t *level)
{
    uint64_t pml4e, pdpe, pde, pte;
    pml4e = get_pml4e_from_pml4_table(vaddr, pml4_base_addr);
    
    *level = PL_NONE;
    /* check if pml4 table entry present */
    if (!(pml4e & X86_PAGE_BIT_PRESENT)) {
        return 0;
    }

    *level = PL_1G;
    pdpe = get_pdpe_from_pdp_table(vaddr, pml4e);

    /* check if pdp table entry present */
    if (!(pdpe & X86_PAGE_BIT_PRESENT)) {
        *last_valid_entry = pml4e;
        return 0;
    }

    *level = PL_2M;
    pde = get_pde_from_pd_table(vaddr, pdpe);

    /* check if pd table entry present */
    if (!(pde & X86_PAGE_BIT_PRESENT)) {
        *last_valid_entry = pdpe;
        return 0;
    }

    /* 2MiB pages */
    if (pde & X86_PAGE_BIT_PS) {
        *last_valid_entry = get_pfn_from_pde(pde);
        return 0;
    }

    *level = PL_4K;
    pte = get_pte_from_ptable(vaddr, pde);

    /* check if page table entry present */
    if (!(pte & X86_PAGE_BIT_PRESENT)) {
        *last_valid_entry = pde;
        return 0;
    }

    /* 4KiB pages */
    if (pde & X86_PAGE_BIT_PS) {
        *last_valid_entry = get_pfn_from_pde(pde);
    }

    return 1;
}

int mmu_map_addr(vaddr_t vaddr, paddr_t paddr, addr_t pml4_base_addr) {
    /* check if vaddr and paddr are cannonical and page aligned */
    if (!mmu_check_vaddr(vaddr) && !mmu_check_paddr(paddr)) {
        return 0;
    }

    /* get required entries */
    uint64_t pml4e, pdpe, pde, pte;
    uint32_t created_pdp = 0, created_pd = 0;

    pml4e = get_pml4e_from_pml4_table(vaddr, pml4_base_addr);
    
    /* check for present flag bit in the entry */
    if (!(pml4e & X86_PAGE_BIT_PRESENT)) {
        /* create a pdp table */
        created_pdp = 1;
    }

    if (!created_pdp) {
        /* in case pdpe was not created retrieve the pdp entry */
        pdpe = get_pdpe_from_pdp_table(vaddr, pml4e);
    } else {
        
    }

    /* update/flush TLB */
}

int mmu_init(void) {
    /* update the address sizes */
    uint32_t addr_size = cpuid_get_addr_size();

    /* Bits 07-00: Physical Address bits */
    /* Bits 15-08: Linear Address bits */    
    uint8_t vaddr_size = (uint8_t)(addr_width & 0xff);
    uint8_t paddr_size = (uint8_t)((addr_width >> 8) & 0xff);

    if (g_vaddr_size < vaddr_size) {
        g_vaddr_size = vaddr_size;
    }

    if (g_paddr_size < paddr_size) {
        g_paddr_size = paddr_size;
    }

    /* flush tlb */
    set_cr3(get_cr3());
}
