#include "asm/current.h"
#include "asm/memory.h"
#include "asm/pgtable-hwdef.h"
#include "asm/pgtable-prot.h"
#include "asm/pgtable.h"
#include "linux/mm.h"
#include "linux/mm_types.h"
#include "linux/sched.h"
#include "linux/types.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "utils.h"

MODULE_LICENSE("GPL");

unsigned long user_va_to_pa(unsigned long user_va) {
    struct page *page = NULL;
    unsigned long pa = 0;
    int result;
    struct mm_struct *mm = current->mm;
    struct vm_area_struct *vma;


    vma = find_vma(mm, user_va);  // Find the VMA for the given user address
    if (!vma || user_va < vma->vm_start) {
        printk(KERN_WARNING "Invalid user space address\n");
        goto out;
    }

    // Pin the page in memory and get the page struct
    result = get_user_pages_fast(user_va, 1, 0, &page);
    if (result <= 0) {
        printk(KERN_WARNING "get_user_pages_fast failed\n");
        goto out;
    }

    // Convert the page struct to a physical address
    pa = page_to_phys(page);
    pa += (user_va & ~PAGE_MASK);  // Add the offset within the page

    put_page(page);  // Decrement the reference count on the page

out:
    return pa;
}

static void walk_page_table(struct mm_struct *mm, unsigned long va) {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page;

    // Get PGD (Page Global Directory) entry
    pgd = pgd_offset(mm, va);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_INFO "Invalid PGD\n");
        return;
    }

    // Get P4D (Page 4th level Directory) entry
    p4d = p4d_offset(pgd, va);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        printk(KERN_INFO "Invalid P4D\n");
        return;
    }

    // Get PUD (Page Upper Directory) entry
    pud = pud_offset(p4d, va);
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_INFO "Invalid PUD\n");
        return;
    }

    // Get PMD (Page Middle Directory) entry
    pmd = pmd_offset(pud, va);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_INFO "Invalid PMD\n");
        return;
    }

    // Get PTE (Page Table Entry)
    pte = pte_offset_map(pmd, va);
    if (pte_none(*pte)) {
        printk(KERN_INFO "Invalid PTE\n");
        pte_unmap(pte);
        return;
    }

    // Get the page structure
    page = pte_page(*pte);
    if (!page) {
        printk(KERN_INFO "Page not found\n");
        pte_unmap(pte);
        return;
    }

    // Get the physical address
    unsigned long pa = page_to_phys(page);
    printk(KERN_INFO "Virtual address: 0x%lx, Physical address: 0x%lx\n", va, pa);

    pte_unmap(pte);
}

uint64_t get_pfn(uint64_t address){
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    uint64_t pfn;
    void * virtual_address;

    int pgd_idx = (address >> PGDIR_SHIFT) & 0x1ff;
    int pud_idx = (address >> PUD_SHIFT) & 0x1ff;
    int pmd_idx = (address >> PMD_SHIFT) & 0x1ff;
    int pte_idx = (address >> PAGE_SHIFT) & 0x1ff;

    walk_page_table(current->mm, address);
    address &= 0xfffffffffffff000;
    pr_info("Looking up addr 0x%llx\n", address);

    pr_info("user to va 0x%lx\n", user_va_to_pa(address));
    pr_info("phyis to virt 0x%lx\n", phys_to_virt(user_va_to_pa(address)));
    

    pgd = current->mm->pgd + pgd_idx;
    pr_info("PGD at 0x%px with value %llx\n", pgd, *pgd);

    pud = phys_to_virt((*(uint64_t*)pgd & PAGE_MASK));
    pud += pud_idx;
    pr_info("PUD at 0x%px with value %llx\n", pud, *pud);


    pmd = phys_to_virt((*(uint64_t*)pud & PAGE_MASK));
    pmd += pmd_idx;
    pr_info("PMD at 0x%px with value %llx\n", pmd, *pmd);

    pte = phys_to_virt((*(uint64_t*)pmd & PAGE_MASK) );
    pte += pte_idx;
    pr_info("PTE at 0x%px with value %llx\n", pte, *pte);


    pfn = (address & 0xfff) + ((*pte).pte & ((1ull << 50) -1 )& 0xffffffffffff000ull) ;
    pr_info("PFN at 0x%llx\n", pfn);

    virtual_address = phys_to_virt(pfn);
    pr_info("Virtual Addr at 0x%px with: ", virtual_address );

    if((*pte).pte & PTE_WRITE){
        pr_info("    -writeable");
    }if ((*pte).pte & PTE_RDONLY){
        pr_info("    -readonly");
    }if ((*pte).pte & PTE_VALID){
        pr_info("    -valid");
    }

    return 0;
}
