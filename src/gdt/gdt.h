
#ifndef GDT_H
#define GDT_H
#include <stdint.h>

// GDT Descriptor entry
struct Gdt
{
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;
} __attribute__((packed));

// Same as Gdt but without bit logic. It is converted to struct Gdt and the processed further  
struct GdtStructured
{
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

void gdt_load(struct Gdt *, int);
void gdt_structured_to_gdt(struct Gdt *, struct GdtStructured *, int);
#endif