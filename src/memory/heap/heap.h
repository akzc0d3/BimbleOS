#ifndef HEAP_H

#include "config.h"
#include <stddef.h>
#include <stdint.h>


#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY * entries;
    size_t total_entries;
};

struct heap 
{
    struct heap_table * table;  // Heap table
    void* saddr;                // Start address for heap memory
};

int heap_create(struct heap*  , void*  , void*  , struct heap_table*  );
void* heap_malloc(struct heap*  , size_t  );
void heap_free(struct heap *heap, void *ptr);

#define HEAP_H
#endif