#ifndef TASKSWITCHSEGMENT_H
#define TASKSWITCHSEGMENT_H


// SS0, SS1, SS2: The Segment Selectors used to load the stack when a privilege level change occurs from a lower privilege level to a higher one.
// ESP0, ESP1, ESP2: The Stack Pointers used to load the stack when a privilege level change occurs from a lower privilege level to a higher one.
// IOPB: I/O Map Base Address Field. Contains a 16-bit offset from the base of the TSS to the I/O Permission Bit Map. 
#include <stdint.h>
struct Tss
{
    uint32_t link;  // Previous Task Link Field. Contains the Segment Selector for the TSS of the previous task.
    uint32_t esp0;  // Kernel stack pointer
    uint32_t ss0;   // Kernel stack segment
    uint32_t esp1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t sr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint32_t iopb;
} __attribute__((packed));

void tss_load(int offset);  // Loads TSS at 'offset' in GDT
#endif