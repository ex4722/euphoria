#include "asm/page-def.h"
#include "linux/mm.h"
#include "linux/printk.h"
#include "linux/slab.h"
#include "euphoria_mm.h"

struct euphoria_mem euphoria_memory;


static const struct vm_operations_struct vm_ops = {
    .close = euphoria_close_vma,
    // Other operations
};

unsigned long alloc_mmap_page(int npages){
    // aligned version of ptr
    unsigned long kmalloc_area;
    int i;

    euphoria_memory.kmalloc_ptr = (unsigned long)kmalloc((2 + npages) * PAGE_SIZE, GFP_KERNEL);
    euphoria_memory.size = (2 + npages) * PAGE_SIZE;
    euphoria_memory.npages = npages;

    if(!euphoria_memory.kmalloc_ptr){
        pr_err("Error allocating mmap page\n");
        return euphoria_memory.kmalloc_ptr;
    }
    kmalloc_area = PAGE_ALIGN(euphoria_memory.kmalloc_ptr);
    pr_info("ptr is 0x%lx and area is 0x%lx\n", euphoria_memory.kmalloc_ptr, kmalloc_area);
    for(i = 0; i < npages * PAGE_SIZE; i+=PAGE_SIZE ){
        // pr_info("Virt Page 0x%lx", virt_to_page(page_vm + i * PAGE_SIZE));
        // pr_info("virt Page 0x%lx", virt_to_phys(page_vm + i * PAGE_SIZE) >> PAGE_OFFSET);
        SetPageReserved(virt_to_page(kmalloc_area+ i));
    }
    pr_info("Allocated %d Pages at 0x%lx\n", npages, kmalloc_area);
    return kmalloc_area;
}

void clear_page_reserved(void * page_vm, int npages){
    int i;
    for(i = 0; i < npages; i++ ){
        ClearPageReserved(virt_to_page(page_vm + i * PAGE_SIZE));
    }
}

void euphoria_close_vma(struct vm_area_struct *vma){
    clear_page_reserved((void*)euphoria_memory.kmalloc_ptr, euphoria_memory.npages);
    kfree((void*)euphoria_memory.kmalloc_ptr);
}
