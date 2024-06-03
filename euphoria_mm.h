#ifndef euphoria_mm
#define euphoria_mm


struct euphoria_mem {
    unsigned long kmalloc_ptr;
    int size;
    int npages;
};
unsigned long alloc_mmap_page(int);
void free_mmap_pages(void *, int);
void euphoria_close_vma(struct vm_area_struct *);
#endif
