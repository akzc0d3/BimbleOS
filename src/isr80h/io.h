
#ifndef ISR80H_IO_H
#define ISR80H_IO_H

struct InterruptFrame;
void* isr80h_command1_print(struct InterruptFrame* frame);
void* isr80h_command2_getkey(struct InterruptFrame* frame);
void* isr80h_command3_putchar(struct InterruptFrame* frame);
#endif