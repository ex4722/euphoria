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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eddie");
MODULE_DESCRIPTION("A simple utils kernel module");

struct file_operations euphoria_fops = {
    .owner = THIS_MODULE,
};
struct miscdevice euphoria_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "euphoria",
    .fops =  &euphoria_fops,
};

static int __init euphoria_start(void){
	pr_info("Loading euphoria module\n");
    if(misc_register(&euphoria_misc) != 0){
        pr_info("Unable to registger misc device\n");
        return -1;
    }else{
        pr_info("Register Euphoria Misc Device Successful\n");
        return 0;
    }
}

static void __exit euphoria_end(void){
    misc_deregister(&euphoria_misc);
	pr_info("Euphoria Module Unloaded\n");
}

module_init(euphoria_start);
module_exit(euphoria_end);
