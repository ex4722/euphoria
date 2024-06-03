/** 
 * @file    euphoria.c 
 * @author  Eddie
 * @brief   Simple module to implemnt basic kernel functions like page walks and mmap handlers
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "linux/fdtable.h"
#include "linux/fs.h"
#include "linux/highmem.h"
#include "linux/miscdevice.h"

#include "euphoria.h"
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
        // pr_info("Register Euphoria Misc Device Successful\n");
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
    return 0;
}

long euphoria_ioctl(struct file *kfile, unsigned int cmd, unsigned long param){
    pr_info("cmd: %d param: 0x%lx\n", cmd, param);
    switch(cmd){
        case EUPHORIA_PFN: 
            return get_pfn(param);
        case EUPHORIA_GET_FD:
            break;
        case EUPHORIA_TESTING:
            break;
        default:
            break;
    }
    return 0;
}

module_init(euphoria_start);
module_exit(euphoria_end);

MODULE_LICENSE("GPL");
