#ifndef KHEAP_H
#define KHEAP_H
 
#include<stddef.h>
#include "memory/memory.h"


void kheap_init();
void* kmalloc(size_t );
void* kzalloc(size_t );
void kfree(void*  );

#endif