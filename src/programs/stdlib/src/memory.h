#ifndef BIMBLEOS_MEMEORY_H
#define BIMBLEOS_MEMEORY_H

#include <stddef.h>

void *memset(void *, int, size_t);
int memcmp(void *, void *, int);
void *memcpy(void *, void *, int);

#endif