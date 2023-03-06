#include "heap.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

 

/**
 * @brief
 *      Validate heap table. Heap table is valid if
 *          table->size (No of entries in table) is correct
 *
 * @param ptr Start address of heap memory
 * @param end End address of heap memory
 * @param table Heap table
 */
static int heap_validate_table(void *ptr, void *end, struct heap_table *table)
{
    int res_status = BIMBLEOS_ALL_OK;

    int table_size = end - ptr;
    int total_blocks = table_size / BIMBLEOS_HEAP_BLOCK_SIZE;

    if (total_blocks != table->total_entries)
    {
        res_status = -EINVARG;
        goto out;
    }

out:
    return res_status;
}

static bool heap_validate_alignment(void *ptr)
{
    return ((unsigned int)ptr % BIMBLEOS_HEAP_BLOCK_SIZE) == 0;
}

/**
 * @brief Return the aligned value of size
 *
 * @param size Size of memory to allocate in heap
 * @return uint32_t Aigned value of size
 */

static uint32_t heap_align_value_to_upper(uint32_t size)
{
    if (size % BIMBLEOS_HEAP_BLOCK_SIZE == 0)
        return size;

    size = size - (size % BIMBLEOS_HEAP_BLOCK_SIZE);
    size += BIMBLEOS_HEAP_BLOCK_SIZE;

    return size;
}

/**
 * @brief
 *      Validate if ptr and end is aligned with BIMBLEOS_HEAP_BLOCK_SIZE
 *      Validate heap table
 *      Initialize struct heap
 *          Set starting address of heap in heap->saddr
 *          Set struct table in heap->table
 *      Initialize heap table
 *          Create memory for heap table and flag them free (Initially all memory is free)
 *
 * @param heap  Pointer to struct heap. This passed heap will be initialized for the caller
 * @param ptr   Start address of heap memory
 * @param end   End address of heap memeory
 * @param table Heap table: Caller will pass initialized table
 *
 * @return
 *      ret == 0    : Everything went good
 *      ret < 0     : Something went wrong. ret represent error code
 */

int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table)
{
    int res_status = BIMBLEOS_ALL_OK;

    // Check if ptr and end are aligned with BIMBLEOS_HEAP_BLOCK_SIZE (4096)
    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end))
    {
        res_status = -EINVARG;
        goto out;
    }

    // Zero out memory for struct heap
    memset(heap, 0, sizeof(struct heap));
    heap->table = table;
    heap->saddr = ptr;

    res_status = heap_validate_table(ptr, end, table);
    if (res_status < 0)
    {
        goto out;
    }

    // Initialize heap table. Flag all memory as free
    int heap_table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total_entries;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, heap_table_size);

out:
    return res_status;
}

//================================== Heap allocation functions =========================================

/**
 * @brief Return the entry type of entry in heap table
 *
 * @param entry
 * @return int
 */
static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0x0f;
}

/**
 * @brief Return address of block
 *
 * @param heap
 * @param block
 * @return void*
 */
void* heap_block_to_address(struct heap *heap, int block)
{
    return heap->saddr + (block * BIMBLEOS_HEAP_BLOCK_SIZE);
}

/**
 * @brief Mark the blocks in heap table as taken
 *
 * @param heap
 * @param start_block
 * @param total_blocks
 */
void heap_mark_blocks_taken(struct heap *heap, int start_block, int total_blocks)
{
    size_t end_block = (start_block + total_blocks) - 1;

    heap->table->entries[start_block] = HEAP_BLOCK_IS_FIRST;
    for(size_t i = start_block; i < end_block; i++){
        heap->table->entries[i] |= HEAP_BLOCK_TABLE_ENTRY_TAKEN |  HEAP_BLOCK_HAS_NEXT;
    }
    heap->table->entries[end_block] |= HEAP_BLOCK_TABLE_ENTRY_TAKEN;
 
}

/**
 * @brief Get the block no. available to use
 *
 * @param heap struct heap
 * @param total_blocks Blocks to allocate
 * @return int Allocated block in heap memory
 */
int heap_get_start_block(struct heap *heap, uint32_t total_blocks)
{
    struct heap_table* table = heap->table;
    int blockCount = 0;
    int blockStart = -1;


    for(size_t i = 0; i < table->total_entries; i++){

        if(heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE){
            blockCount = 0;
            blockStart = -1;
            continue;
        }
        if(blockStart == -1){
            blockStart = i;
        }
        blockCount++;
        if(blockCount == total_blocks){
            break;
        }

    }
    
    if(blockStart == -1){
        return -ENOMEM;
    }

    return blockStart;
}

/**
 * @brief Allocate blocks of memory in heap
 *
 * @param heap struct heap
 * @param total_blocks Total blocks of heap memeory to allocate
 * @return void* void* Pointer to allocated heap memory
 */
void *heap_malloc_blocks(struct heap *heap, uint32_t total_blocks)
{
    void *address = 0;

    int start_block = heap_get_start_block(heap,total_blocks);

    if(start_block < 0){
        goto out;
    }

    address = heap_block_to_address(heap,start_block);

    
    heap_mark_blocks_taken(heap,start_block,total_blocks);
    out:
    return address;
}

/**
 * @brief Allocate bytes of memory in heap
 *
 * @param heap struct heap
 * @param total_size Bytes to allocate in heap ( total_size will be aligned to BIMBLEOS_HEAP_BLOCK_SIZE)
 * @return void* Pointer to allocated heap memory
 */
void* heap_malloc(struct heap *heap, size_t total_size)
{

    size_t aligned_size = heap_align_value_to_upper(total_size);
    uint32_t total_blocks = aligned_size / BIMBLEOS_HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, total_blocks);
}


//================================== Heap de-allocation functions =========================================

int heap_address_to_block(struct heap* heap, void* address)
{
    return ((int)(address - heap->saddr)) / BIMBLEOS_HEAP_BLOCK_SIZE;
}



void heap_mark_blocks_free(struct heap* heap, int starting_block)
{
    struct heap_table* table = heap->table;
    for (size_t i = starting_block; i <  table->total_entries; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
            break;
        }
    }
}
void heap_free(struct heap *heap, void *ptr)
{
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}