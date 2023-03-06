#ifndef BIMBLEOS_STDLIB_H
#define BIMBLEOS_STDLIB_H
#include <stddef.h>


void* malloc(size_t size);
void free(void* size);
char* itoa(int i);

#endif