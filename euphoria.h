/** 
 * @file    euphoria.h
 * @author  Eddie
 * @brief   Header file for euphoria including macros 
*/


#define pr_fmt(fmt) "[%s:%s] " fmt, KBUILD_MODNAME, __func__
#define pr_pointer(pointer) pr_info("%s = 0x%px\n", #pointer, pointer)
