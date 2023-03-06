#include "kernel.h"
#include "idt.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "io/io.h"
#include "task/task.h"
#include "task/process.h"


struct idt_desc idt_descriptors[BIMBLEOS_TOTAL_INTERRUPTS];     // Memory space for IDT
struct idtr_desc idtr_descriptor;                               // Memory space for IDTR


extern void* interrupt_pointer_table[BIMBLEOS_TOTAL_INTERRUPTS];
static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[BIMBLEOS_TOTAL_INTERRUPTS];


static ISR80H_COMMAND isr80h_commands[BIMBLEOS_MAX_ISR80H_COMMANDS];



extern void isr80h_wrapper();

 



void interrupt_handler(int interrupt, struct InterruptFrame* frame)
{
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0)
    {
        task_current_save_state(frame);
        interrupt_callbacks[interrupt](frame);
    }

    task_page();
    outb(0x20, 0x20);   // Acknowledge interrupt
}

extern void* idt_load(struct idtr_desc * ptr);          // Implementation of this function is in assembly          

// INT 0 handler
void divide_by_zero(){
    print("Divide by zero");
      
 }



void idt_handle_exception()
{
    print("Exception occured");
    process_terminate(task_current()->process);
    task_next();
}
// Sets a handler for interrupt number
void idt_set(int interrupt_no, void * address){
    struct idt_desc * desc =  &idt_descriptors[interrupt_no];

    desc->offset_1 = (uint32_t)address & 0x0000FFFF;
    desc->selector = KERNAL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE;
    desc->offset_2 = (uint32_t)address >> 16;
}
void idt_clock()
{
    outb(0x20, 0x20);

    // Switch to the next task
    task_next();
}

// Initialize IDT
void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));    // Zero out the memory descriptor table
    
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;    // Set IDTR
    idtr_descriptor.base = (uint32_t) idt_descriptors;


    for(int i = 0; i < BIMBLEOS_TOTAL_INTERRUPTS ; i++){
        idt_set(i,interrupt_pointer_table[i]);
    }

    idt_set(0x0,divide_by_zero);                                // Set INT0 handler
    idt_set(0x80,isr80h_wrapper);                               // Set INT80H handler
    
    for (size_t i = 0; i < 0x20; i++)
    {
        idt_register_interrupt_callback(i,idt_handle_exception);
    }
    
    idt_register_interrupt_callback(0x20, idt_clock);
    
    idt_load(&idtr_descriptor);                             // Load interrupt descriptor table
}



int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    if (interrupt < 0 || interrupt >= BIMBLEOS_TOTAL_INTERRUPTS)
    {
        return -EINVARG;
    }

    interrupt_callbacks[interrupt] = interrupt_callback;
    return 0;
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= BIMBLEOS_MAX_ISR80H_COMMANDS)
    {
        panic("The command is out of bounds\n");
    }

    if (isr80h_commands[command_id])
    {
        panic("Your attempting to overwrite an existing command\n");
    }

    isr80h_commands[command_id] = command;
}


/**
 * @brief Invoke appropriate handle function for INT80H and return a pointer to result.
 * 
 * @param command 
 * @param frame 
 * @return void* 
 */
void* isr80h_handle_command(int command, struct InterruptFrame* frame)
{
    void* result = 0;

    if(command < 0 || command >= BIMBLEOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        return 0;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func)
    {
        return 0;
    }

    result = command_func(frame);
    return result;
}

/**
 * @brief 1) Switches to kernel page directory 
 *        2) Saves current running task.
 *        3) Gets the interrupt result ( by calling "isr80h_handle_command" )
 *        4) Switches to current task's page directory
 *        5) Returns pointer to interrupt result
 * 
 * @param command 
 * @param frame 
 * @return void* 
 */
void* isr80h_handler(int command, struct InterruptFrame* frame)
{
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
} 
