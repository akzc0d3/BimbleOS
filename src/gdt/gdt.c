#include "gdt.h"
#include "kernel.h"

void encodeGdtEntry(uint8_t* target, struct GdtStructured source)
{
    if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF))
    {
        panic("encodeGdtEntry: Invalid argument\n");
    }

    target[6] = 0x40;   // Set 'DB' flag,  If clear (0), the descriptor defines a 16-bit protected mode segment. If set (1) it defines a 32-bit protected mode segment. A GDT can have both 16-bit and 32-bit selectors at once. 
    if (source.limit > 65536)
    {
        source.limit = source.limit >> 12;
        target[6] = 0xC0;   // Set the Granuality flag
    }

    // Encodes the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Set the type
    target[5] = source.type;

}

void gdt_structured_to_gdt(struct Gdt* gdt, struct GdtStructured* structured_gdt, int total_entires)
{
    for (int i = 0; i < total_entires; i++)
    {
        encodeGdtEntry((uint8_t*)&gdt[i], structured_gdt[i]);
    }
}
 