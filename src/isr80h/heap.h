#ifndef ISR80H_HEAP_H
#define ISR80H_HEAP_H

 
struct InterruptFrame;
void* isr80h_command4_malloc(struct InterruptFrame* frame);
void* isr80h_command5_free(struct InterruptFrame* frame);

#endif