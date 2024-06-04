/** 
 * @file    euphoria.h
 * @author  Eddie
 * @brief   Header file for euphoria including macros 
*/

#define EUPHORIA_PFN 0x1337
#define EUPHORIA_GET_FD 0xdeadbeef
#define EUPHORIA_TESTING 0xcafebabe
#define EUPHORIA_NO_VMA 0xba11

#undef pr_fmt
#define pr_fmt(fmt) "[%s:%s] " fmt, KBUILD_MODNAME, __func__
#define pr_pointer(pointer) pr_info("%s = 0x%px\n", #pointer, pointer)

int euphoria_mmap(struct file *, struct vm_area_struct *);
int euphoria_open(struct inode *, struct file *);
long euphoria_ioctl(struct file *, unsigned int, unsigned long);

