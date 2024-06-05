#include "asm/current.h"
#include "asm/memory.h"
#include "asm/pgtable.h"
#include "linux/fdtable.h"
#include "linux/mm.h"
#include "linux/mm_types.h"
#include "linux/sched.h"
#include "linux/types.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "utils.h"

MODULE_LICENSE("GPL");

uint64_t get_pfn(uint64_t address){
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    uint64_t physical_address;
    void * virtual_address;

    int pgd_idx = (address >> PGDIR_SHIFT) & 0x1ff;
    int pud_idx = (address >> PUD_SHIFT) & 0x1ff;
    int pmd_idx = (address >> PMD_SHIFT) & 0x1ff;
    int pte_idx = (address >> PAGE_SHIFT) & 0x1ff;

    pgd = current->mm->pgd + pgd_idx;
    pr_info("PGD at 0x%px with value 0x%llx\n", pgd, (*pgd).pgd);

    pud = phys_to_virt((*(uint64_t*)pgd & PAGE_MASK));
    pud += pud_idx;
    pr_info("PUD at 0x%px with value 0x%llx\n", pud, (*pud).pud);


    pmd = phys_to_virt((*(uint64_t*)pud & PAGE_MASK));
    pmd += pmd_idx;
    pr_info("PMD at 0x%px with value 0x%llx\n", pmd, (*pmd).pmd);

    pte = phys_to_virt((*(uint64_t*)pmd & PAGE_MASK) );
    pte += pte_idx;
    pr_info("PTE at 0x%px with value 0x%llx\n", pte, (*pte).pte);


    physical_address = (address & 0xfff) + ((*pte).pte & ((1ull << 50) -1 )& 0xffffffffffff000ull) ;

    virtual_address = phys_to_virt(physical_address);

    pr_info("Physical Address at 0x%llx, Virtual Addr at 0x%llx with flags: %llu\n", physical_address, (uint64_t)virtual_address, (*pte).pte & 0xfff);

    if ((*pte).pte & PTE_VALID)      printk(KERN_INFO "  - Valid\n");
    if ((*pte).pte & PTE_USER)       printk(KERN_INFO "  - User\n");
    if ((*pte).pte & PTE_RDONLY)     printk(KERN_INFO "  - Readonly\n");
    if ((*pte).pte & PTE_AF)         printk(KERN_INFO "  - Accessed\n");
    if ((*pte).pte & PTE_SHARED)     printk(KERN_INFO "  - Shareable\n");
    if ((*pte).pte & PTE_CONT)       printk(KERN_INFO "  - Contiguous\n");
    if ((*pte).pte & PTE_PXN)        printk(KERN_INFO "  - Privileged Execute Never (PXN)\n");
    if ((*pte).pte & PTE_UXN)        printk(KERN_INFO "  - Unprivileged Execute Never (UXN)\n");

    return physical_address >> PAGE_SHIFT;
}
void get_file_struct(void){
    struct file * cur;
    struct fdtable * fdt;
    int index;

    fdt = rcu_dereference_check_fdtable((current->files), (current->files)->fdt);

    spin_lock(&current->files->file_lock);
    pr_info("Current process %s files count %d\n", current->comm, atomic_read(&current->files->count));
    for(index = 0; index < fdt->max_fds; index++){
        cur = fdt->fd[index];
        if(cur == NULL){
            continue;
        }
        pr_info("Filename: %s Inode: %lu refcount: %lu\n", cur->f_path.dentry->d_name.name,cur->f_inode->i_ino, atomic_long_read(&cur->f_count));
    }
    spin_unlock(&current->files->file_lock);
}


// shitty version of remap_pfn_range
// currently it maps in page at the addres provided that is one page, page is rw
uint64_t euphoria_insert_page_entry(uint64_t address){
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    void * virtual_address;

    int pgd_idx = (address >> PGDIR_SHIFT) & 0x1ff;
    int pud_idx = (address >> PUD_SHIFT) & 0x1ff;
    int pmd_idx = (address >> PMD_SHIFT) & 0x1ff;
    int pte_idx = (address >> PAGE_SHIFT) & 0x1ff;

    virtual_address = page_to_virt( alloc_pages(GFP_KERNEL, 0));
    pr_info("New Backing Page allocated at 0x%px\n", virtual_address);

    pgd = current->mm->pgd + pgd_idx;
    pr_info("PGD at 0x%px with value 0x%llx\n", pgd, (*pgd).pgd);
    if((*pgd).pgd == 0){
        // flags r for userspace pages
        (*pgd).pgd = page_to_phys(alloc_pages(GFP_KERNEL | __GFP_ZERO|__GFP_ACCOUNT, 0)) | 3;
        pr_info("PGD empty so replacing with 0x%llx\n", (*pgd).pgd);
    }
    pud = phys_to_virt((*(uint64_t*)pgd & PAGE_MASK));
    pud += pud_idx;
    pr_info("PUD at 0x%px with value 0x%llx\n", pud, (*pud).pud);


    if((*pud).pud == 0){
        // flags r for userspace pages
        (*pud).pud = page_to_phys(alloc_pages(GFP_KERNEL | __GFP_ZERO|__GFP_ACCOUNT, 0)) | 3;
        pr_info("PUD empty so replacing with 0x%llx\n", (*pud).pud);
    }
    pmd = phys_to_virt((*(uint64_t*)pud & PAGE_MASK));
    pmd += pmd_idx;
    pr_info("PMD at 0x%px with value 0x%llx\n", pmd, (*pmd).pmd);

    if((*pmd).pmd == 0){
        // flags r for userspace pages
        (*pmd).pmd = page_to_phys(alloc_pages(GFP_KERNEL | __GFP_ZERO|__GFP_ACCOUNT, 0)) | 3;
        pr_info("PMD empty so replacing with 0x%llx\n", (*pmd).pmd);
    }
    pte = phys_to_virt((*(uint64_t*)pmd & PAGE_MASK) );
    pte += pte_idx;
    (*pte).pte = virt_to_phys(virtual_address);
    (*pte).pte |= PTE_VALID | PTE_USER | PTE_UXN | PTE_PXN | PTE_AF | PTE_SHARED | PTE_WRITE | PTE_SPECIAL | PTE_TABLE_BIT;
    pr_info("PTE at 0x%px with value 0x%llx\n", pte, (*pte).pte);

    return 0;
};
