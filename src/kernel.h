#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define ERROR(value) (void *)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)





void print(const char *);
void terminal_writechar(char character, char color);
void kernel_main();
void panic(const char *);

void kernel_page();
void kernel_registers();


#endif