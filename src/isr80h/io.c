#include "io.h"
#include "task/task.h"
#include "kernel.h"
#include "keyboard/keyboard.h"
void* isr80h_command1_print(struct InterruptFrame* frame)
{
    void* user_space_msg_buffer = task_get_stack_item(task_current(), 0);
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));

    print(buf);
    return 0;
}
 

void* isr80h_command2_getkey(struct InterruptFrame* frame)
{
    char c = keyboard_pop();
    return (void*)((int)c);
}


void* isr80h_command3_putchar(struct InterruptFrame* frame)
{
    char c = (char)(int) task_get_stack_item(task_current(), 0);
    terminal_writechar(c, 15);
    return 0;
}