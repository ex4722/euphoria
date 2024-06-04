/** 
 * @file    euphoria.c 
 * @author  Eddie
 * @brief   Simple module to implemnt basic kernel functions like page walks and mmap handlers
*/
#include "linux/mm.h"
#include "linux/slab.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "linux/fdtable.h"
#include "linux/fs.h"
#include "linux/miscdevice.h"

#include "euphoria.h"
#include "euphoria_mm.h"
#include "utils.h"

MODULE_AUTHOR("Eddie");
MODULE_DESCRIPTION("A simple utils kernel module");

struct file_operations euphoria_fops = {
    .owner = THIS_MODULE,
    .open = euphoria_open,
    .mmap = euphoria_mmap,
    .compat_ioctl = euphoria_ioctl,
    .unlocked_ioctl = euphoria_ioctl,
};

struct miscdevice euphoria_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "euphoria",
    .fops =  &euphoria_fops,
};

static int __init euphoria_start(void){
	printk(KERN_ALERT "Loading euphoria module\n");
	pr_info("Loading euphoria module\n");
    if(misc_register(&euphoria_misc) != 0){
        pr_info("Unable to registger misc device\n");
        return -1;
    }else{
        printk(KERN_ALERT "Register Euphoria Misc Device Successful\n");
        return 0;
    }
}

static void __exit euphoria_end(void){
    misc_deregister(&euphoria_misc);
	pr_info("Euphoria Module Unloaded\n");
}

int euphoria_open(struct inode *inode, struct file *file){
    pr_info("Euphoria device opened successfully\n");
    return 0;
}

int euphoria_mmap(struct file *kfile, struct vm_area_struct *vma){
    void * page_vm;
    long int vm_size;
    int ret;
    vm_size = vma->vm_end - vma->vm_start;
    vm_size = 0x2000;
    pr_info("Euphoria MMAP called with start:0x%lx end:0x%lx length:0x%lx flags:0x%lx\n", vma->vm_start, vma->vm_end, vm_size, vma->vm_flags);
    page_vm = (void *)alloc_mmap_page(vm_size / PAGE_SIZE);
    vma->vm_ops = &euphoria_vm_ops;

    pr_info("Mapping page to %llx\n",virt_to_phys((void *)page_vm) >> PAGE_SHIFT );
    ret = remap_pfn_range(vma,vma->vm_start, virt_to_phys((void *)page_vm) >> PAGE_SHIFT, vm_size, vma->vm_page_prot);
    if(ret < 0){
        pr_err("Could not remap the area\n");
        return -EIO;
    }
    return 0;
}

void euphoria_test(unsigned long parm){
        struct vm_area_struct *vma;
        void * page_vm;
        vma = kzalloc(sizeof(*vma), GFP_KERNEL);
        vma->vm_start = parm;
        vma->vm_end = parm + PAGE_SIZE;
        vma->vm_mm = current->mm;

        vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(VM_READ | VM_WRITE | VM_MAYREAD | VM_MAYWRITE | VM_PFNMAP | VM_SHARED));
        vma->vm_page_prot.pgprot &= ~(1 << 7);
        vma->vm_flags = VM_READ | VM_WRITE | VM_MAYREAD | VM_MAYWRITE | VM_PFNMAP | VM_SHARED;

        page_vm = (void *)alloc_mmap_page(1);
        pr_info("vma start aka param is at 0x%lx with page frame number at 0x%llx Page Prot: %llx\n", vma->vm_start, virt_to_phys((void *)page_vm) >> PAGE_SHIFT , vma->vm_page_prot.pgprot);
        remap_pfn_range(vma,vma->vm_start, virt_to_phys((void *)page_vm) >> PAGE_SHIFT, PAGE_SIZE*2, vma->vm_page_prot);
}

long euphoria_ioctl(struct file *kfile, unsigned int cmd, unsigned long param){
    pr_info("cmd: %x param: 0x%lx\n", cmd, param);
    switch(cmd){
        case EUPHORIA_PFN: 
            return get_pfn(param);
        case EUPHORIA_GET_FD:
            get_file_struct();
            break;
        case EUPHORIA_TESTING:
            euphoria_test(param);
            break;
        case EUPHORIA_NO_VMA:
            euphoria_create_backing_without_vma(param);
            break;
        default:
            break;
    }
    return 0;
}

module_init(euphoria_start);
module_exit(euphoria_end);

MODULE_LICENSE("GPL");
