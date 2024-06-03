#include "asm/current.h"
#include "asm/memory.h"
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
    pr_info("PGD at 0x%px with value 0x%llx\n", pgd, *pgd);

    pud = phys_to_virt((*(uint64_t*)pgd & PAGE_MASK));
    pud += pud_idx;
    pr_info("PUD at 0x%px with value 0x%llx\n", pud, *pud);


    pmd = phys_to_virt((*(uint64_t*)pud & PAGE_MASK));
    pmd += pmd_idx;
    pr_info("PMD at 0x%px with value 0x%llx\n", pmd, *pmd);

    pte = phys_to_virt((*(uint64_t*)pmd & PAGE_MASK) );
    pte += pte_idx;
    pr_info("PTE at 0x%px with value 0x%llx\n", pte, *pte);


    physical_address = (address & 0xfff) + ((*pte).pte & ((1ull << 50) -1 )& 0xffffffffffff000ull) ;

    virtual_address = phys_to_virt(physical_address);

    pr_info("Physical Address at 0x%llx, Virtual Addr at 0x%llx with: \n", physical_address, (uint64_t)virtual_address);

    if((*pte).pte & PTE_WRITE){
        pr_info("    -writeable");
    }if ((*pte).pte & PTE_VALID){
        pr_info("    -valid");
    }

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
