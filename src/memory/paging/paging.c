#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"



static uint32_t *current_directory = 0;


/**
 * @brief Create a linear page directory
 * 
 * @param flags 
 * @return struct PageDirectory_4GB* 
 */
struct PageDirectory_4GB* paging_new(uint8_t flags){

    uint32_t * directory = kzalloc(sizeof(uint32_t)* PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    
    for (size_t i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++){
        uint32_t *entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

        for (size_t j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
        {
            entry[j] =  offset + ((j * PAGING_PAGE_SIZE) | flags) ;
            
        }
        offset+=PAGING_PAGE_SIZE * PAGING_TOTAL_ENTRIES_PER_TABLE;
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }

    struct PageDirectory_4GB * pageDirectory = kzalloc(sizeof(struct PageDirectory_4GB));
    pageDirectory->directoryEntry = directory;

    return pageDirectory;
}

uint32_t *paging_get_directory(struct PageDirectory_4GB *pageDirectory)
{
    return pageDirectory->directoryEntry;
}

/**
 * @brief Switch to new page directory
 * 
 * @param directory 
 */
void paging_switch(struct PageDirectory_4GB *directory)
{
    paging_load_directory(directory->directoryEntry);
    current_directory = directory->directoryEntry;
}

bool paging_is_aligned(void* addr){

    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0 ;
} 

void paging_free(struct PageDirectory_4GB *chunk)
{
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directoryEntry[i];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000);
        kfree(table);
    }

    kfree(chunk->directoryEntry);
    kfree(chunk);
}

/**
 * @brief Get directory and table index given a virtual address
 * 
 * @param virt_addr 
 * @param directory_index_out Directory index
 * @param table_index_out Table index
 * @return int 
 */
int paging_get_indexes(void* virt_addr, uint32_t* directory_index_out, uint32_t* table_index_out ){
    int res = 0;
    if(!paging_is_aligned(virt_addr)){
        res =  -EINVARG;
        goto out;
    }

    *directory_index_out = (uint32_t)virt_addr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    *table_index_out = ((uint32_t)virt_addr  % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);
    
    out:
    
    return res;
}

/**
 * @brief Modify page directory
 * 
 * @param directory Pointer to page directory
 * @param virt_addr The virtual address
 * @param val The virt_addr will point to this value
 * @return int 
 */
int paging_set(uint32_t* directory, void* virt_addr, uint32_t val){


    if (!paging_is_aligned(virt_addr))
    {
        return -EINVARG;
    }
    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    int res = paging_get_indexes(virt_addr, &directory_index, &table_index);
    
    if (res < 0)
    {
        return res;
    }

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xfffff000);
    table[table_index] = val;

    return 0;
}

void* paging_align_address(void* ptr)
{
    if ((uint32_t)ptr % PAGING_PAGE_SIZE)
    {
        return (void*)((uint32_t)ptr + PAGING_PAGE_SIZE - ((uint32_t)ptr % PAGING_PAGE_SIZE));
    }
    
    return ptr;
}

int paging_map(struct PageDirectory_4GB* directory, void* virt, void* phys, int flags)
{
    if (((unsigned int)virt % PAGING_PAGE_SIZE) || ((unsigned int) phys % PAGING_PAGE_SIZE))
    {
        return -EINVARG;
    }

    return paging_set(directory->directoryEntry, virt, (uint32_t) phys | flags);
}

int paging_map_range(struct PageDirectory_4GB* directory, void* virt, void* phys, int count, int flags)
{
    int res = 0;
    for (int i = 0; i < count; i++)
    {
        res = paging_map(directory, virt, phys, flags);
        if (res < 0)
            break;
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
    }

    return res;
}

int paging_map_to(struct PageDirectory_4GB *directory, void *virt, void *phys, void *phys_end, int flags)
{
    int res = 0;
    if ((uint32_t)virt % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }
    if ((uint32_t)phys % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }
    if ((uint32_t)phys_end % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }

    if ((uint32_t)phys_end < (uint32_t)phys)
    {
        res = -EINVARG;
        goto out;
    }

    uint32_t total_bytes = phys_end - phys;
    int total_pages = total_bytes / PAGING_PAGE_SIZE;
    res = paging_map_range(directory, virt, phys, total_pages, flags);
out:
    return res;
}

void* paging_get_physical_address(uint32_t* directory, void* virt)
{
    void* virt_addr_new = (void*) paging_align_to_lower_page(virt);
    void* difference = (void*)((uint32_t) virt - (uint32_t) virt_addr_new);
    return (void*)((paging_get(directory, virt_addr_new) & 0xfffff000) + difference);
}

uint32_t paging_get(uint32_t* directory, void* virt)
{
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(virt, &directory_index, &table_index);
    
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    return table[table_index];
}

void* paging_align_to_lower_page(void* addr)
{
    uint32_t _addr = (uint32_t) addr;
    _addr -= (_addr % PAGING_PAGE_SIZE);
    return (void*) _addr;
}


