#ifndef euphoria_mm
#define euphoria_mm


struct euphoria_mem {
    unsigned long kmalloc_ptr;
    int size;
    int npages;
};
unsigned long alloc_mmap_page(int);
void clear_page_reserved(void *, int);
void euphoria_close_vma(struct vm_area_struct *);
void euphoria_create_backing_without_vma(uint64_t address);

extern const struct vm_operations_struct euphoria_vm_ops;
#endif
