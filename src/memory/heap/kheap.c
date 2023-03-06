#include "kernel.h"
#include "kheap.h"
#include "heap.h"
#include "config.h"
 
struct heap kernel_heap;
struct heap_table kernel_heap_table;


/**
 * @brief Inilializes heap_table : 
 *        Calls heap_create to create heap
 */
 
void kheap_init(){
    
    size_t total_heap_entries = 104857600 / 4096;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)BIMBLEOS_HEAP_TABLE_ADDRESS;
    kernel_heap_table.total_entries = total_heap_entries;

    void* end = (void*)BIMBLEOS_HEAP_ADDRESS + BIMBLEOS_HEAP_SIZE_BYTES;


    int res_status =  heap_create(&kernel_heap, (void*)BIMBLEOS_HEAP_ADDRESS, end ,&kernel_heap_table );
    
    if(res_status < 0){
        print("Failed to create heap\n");
    }

}

void* kmalloc(size_t size){
    return heap_malloc(&kernel_heap,size);
}

void* kzalloc(size_t size){
    void* ptr = kmalloc(size);

    if(!ptr)
        return 0;

    memset(ptr,0x00,size);
    return ptr;
    
}
void kfree(void* ptr){
    heap_free(&kernel_heap,ptr);
}