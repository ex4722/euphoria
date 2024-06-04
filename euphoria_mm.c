#include "asm/page-def.h"
#include "linux/mm.h"
#include "linux/printk.h"
#include "linux/slab.h"
#include "euphoria_mm.h"

struct euphoria_mem euphoria_memory;


const struct vm_operations_struct euphoria_vm_ops = {
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

// Alloctes one page at address with no backing and rdwd permissions
void euphoria_create_backing_without_vma(uint64_t address){
        struct vm_area_struct *vma;
        vma = kzalloc(sizeof(*vma), GFP_KERNEL);
        vma->vm_start = address;
        vma->vm_end = address+ PAGE_SIZE;
        vma->vm_mm = current->mm;

        vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(VM_READ | VM_WRITE | VM_MAYREAD | VM_MAYWRITE | VM_PFNMAP | VM_SHARED));
        // remote the rdonly bit 
        vma->vm_page_prot.pgprot &= ~(1 << 7);
        vma->vm_flags = VM_READ | VM_WRITE | VM_MAYREAD | VM_MAYWRITE | VM_PFNMAP | VM_SHARED;

        void * page_vm = (void *)alloc_mmap_page(1);
        pr_info("vma start aka param is at 0x%lx with page frame number at 0x%llx Page Prot: %x\n", vma->vm_start, virt_to_phys((void *)page_vm) >> PAGE_SHIFT , vma->vm_page_prot);
        remap_pfn_range(vma,vma->vm_start, virt_to_phys((void *)page_vm) >> PAGE_SHIFT, PAGE_SIZE*2, vma->vm_page_prot);
}
