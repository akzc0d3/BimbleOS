#ifndef TASK_H
#define TASK_H

#include "config.h"
#include "memory/paging/paging.h"



struct InterruptFrame;
struct Registers
{

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct Task
{
    // The process of task
    struct Process *process;

    struct PageDirectory_4GB *page_directory;

    // The registers of the task when the task is not running
    struct Registers registers;

    // The next task in the linked list
    struct Task *next;

    // Previous task in the linked list
    struct Task *prev;
};

struct Task *task_get_next();
int task_free(struct Task *);
struct Task *task_new();
struct Task *task_current();
void restore_general_purpose_registers(struct Registers *);
void task_return(struct Registers *); // Drops us on userland
int task_page();
int task_page_task(struct Task* task);
int task_switch(struct Task *);
void task_run_first_ever_task();
void user_registers();
void task_save_state(struct Task *, struct InterruptFrame *);
void task_current_save_state(struct InterruptFrame *);
int copy_string_from_task(struct Task* task, void* virtual, void* phys, int max);
void* task_get_stack_item(struct Task* task, int index);
void* task_virtual_address_to_physical(struct Task* task, void* virtual_address);
void task_next();

#endif