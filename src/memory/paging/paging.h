#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>

#define PAGING_CACHE_DISABLED 0b00010000  // If set, disables page caching
#define PAGING_WRITE_THROUGH 0b00001000   // If set, write through caching enabled, else write back caching enabled
#define PAGING_ACCESS_FROM_ALL 0b00000100 // If set, page accesible through all ring level, else only accessible to supervisor ring
#define PAGING_IS_WRITEABLE 0b00000010    // If set, reqding and writing is enabled, else only readable. WP bit in CR0 acn allow eriting in all ases for supervisor
#define PAGING_IS_PRESENT 0b00000001      // If set, implies page exist in real memory

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

// Data structure pointing to a page directory
struct PageDirectory_4GB
{
    uint32_t *directoryEntry;
};

struct PageDirectory_4GB *paging_new(uint8_t flags);
uint32_t *paging_get_directory(struct PageDirectory_4GB *pageDirectory);
void paging_switch(struct PageDirectory_4GB *directory);
bool paging_is_aligned(void *addr);
void paging_free(struct PageDirectory_4GB *chunk);
int paging_get_indexes(void *virt_addr, uint32_t *directory_index_out, uint32_t *table_index_out);
void enable_paging();
int paging_set(uint32_t *directory, void *virt_addr, uint32_t val);
void *paging_align_address(void *ptr);
int paging_map(struct PageDirectory_4GB *directory, void *virt, void *phys, int flags);
int paging_map_to(struct PageDirectory_4GB *directory, void *virt, void *phys, void *phys_end, int flags);
uint32_t paging_get(uint32_t *directory, void *virt);
void paging_load_directory(uint32_t * directory);
void* paging_align_to_lower_page(void* addr);
void* paging_get_physical_address(uint32_t* directory, void* virt);

#endif