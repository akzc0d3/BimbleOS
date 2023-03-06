#ifndef ISR80H_PROCESS_H
#define ISR80H_PROCESS_H


struct InterruptFrame;
void* isr80h_command6_process_load_start(struct InterruptFrame* frame);
void* isr80h_command7_invoke_system_command(struct InterruptFrame* frame);
void* isr80h_command8_get_program_arguments(struct InterruptFrame* frame);
void* isr80h_command9_exit(struct InterruptFrame* frame);

#endif