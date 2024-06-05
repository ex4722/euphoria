#ifndef UTILS_H

#define UTILS_H
uint64_t get_pfn(uint64_t address);
void get_file_struct(void);

uint64_t insert_page_entry(uint64_t addr);

#endif
