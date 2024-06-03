/** 
 * @file    euphoria.c 
 * @author  Eddie
 * @brief   Simple module to implemnt basic kernel functions like page walks and mmap handlers
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> 
#include "euphoria.h"
  
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Eddie"); 
MODULE_DESCRIPTION("A simple utils kernel module"); 
  
static int __init hello_start(void) 
{ 
    pr_info("Loading euphoria module\n"); 
    return 0; 
} 
  
static void __exit hello_end(void) 
{ 
    pr_info("Euphoria Module exiting\n"); 
} 
  
module_init(hello_start); 
module_exit(hello_end); 
