#include "kernel.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "fs/file.h"
#include "gdt/gdt.h"
#include "keyboard/keyboard.h"
#include "config.h"
#include "status.h"
#include "task/tss.h"
#include "task/process.h"
#include "isr80h/isr80h.h"
#include <stdint.h>
#include <stddef.h>



static struct PageDirectory_4GB *kernelPageDirectory = 0;
struct Gdt gdt_real[BIMBLEOS_TOTAL_GDT_SEGMENT];
struct Tss tss;


struct GdtStructured gdt_structured[BIMBLEOS_TOTAL_GDT_SEGMENT]  = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                    // NULL Segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A},              // Kernel Mode Code Segment 
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92},              // Kernel Mode Data Segment 
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xFA},              // User Mode Code Segment 
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2},              // User Mode Data Segment 
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0x89},   // Task State Segment 
};

uint16_t *video_mem = 0;
int terminal_row = 0;
int terminal_column = 0;



// Return character and color in little endian that can be placed in video memory
uint16_t terminal_make_char(char character, char color)
{
    return (color << 8) | character;
}


void terminal_backspace()
{
    if (terminal_row == 0 && terminal_column == 0)
    {
        return;
    }

    if (terminal_column == 0)
    {
        terminal_row -= 1;
        terminal_column = VGA_WIDTH;
    }

    terminal_column -=1;
    terminal_writechar(' ', 15);
    terminal_column -=1;
}


/**
 * @brief Place character and color in video mem on x row and y column
 * 
 * @param x 
 * @param y 
 * @param character 
 * @param color 
 */
void terminal_putchar(int x, int y, char character, char color)
{
    video_mem[(VGA_WIDTH * y) + x] = terminal_make_char(character, color);
}


/**
 * @brief Prints a character on screen
 * 
 * @param character 
 * @param color 
 */
void terminal_writechar(char character, char color)
{
    if (character == '\n')
    {
        terminal_row += 1;
        terminal_column = 0;
        return;
    }

    if (character == 0x08)
    {
        terminal_backspace();
        return;
    }
    terminal_putchar(terminal_column, terminal_row, character, color);
    terminal_column++;

    if (terminal_column >= VGA_WIDTH)
    {

        terminal_row++;
        terminal_column = 0;
    }
}


/**
 * @brief Print a string on screen
 * 
 * @param str 
 */
void print(const char *str)
{

    for (int i = 0; i < strlen(str); i++)
    {
        terminal_writechar(str[i], 15);
    }
}


/**
 * @brief Initialize video memory (Blank the screen by filling it with space)
 * 
 */
void terminal_initialize()
{
    terminal_row = 0;
    terminal_column = 0;
    video_mem = (uint16_t *)(0xB8000);

    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x, y, ' ', 0);
        }
    }
}


void panic(const char * msg){
    print(msg);
    while (1){}
    
}


/**
 * @brief 1) Switches to kernel page directory,
 *        2) Loads kernel register
 * 
 */
void kernel_page()
{
    kernel_registers();
    paging_switch(kernelPageDirectory);
}


/**
 * @brief Kernel main function
 * 
 */
void kernel_main()
{
 
    // Initialize Video memeory
    terminal_initialize();

    // Setup GDT
    memset(gdt_real,0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real,gdt_structured,BIMBLEOS_TOTAL_GDT_SEGMENT);
    gdt_load(gdt_real,sizeof(gdt_real));  
    
    
    kheap_init();
    fs_init();
    disk_search_and_init();
    idt_init();

    // Setup Task State Segment
    memset(&tss,0x00, sizeof(tss));
    tss.esp0 = 0x600000;                // Processor will set the kernel stack to this address when switching from user land to kernel land
    tss.ss0 = KERNAL_DATA_SELECTOR;
    tss_load(0x28);                     // 0x28 is offset of TSS in GDT


    // Setup kernel page directory
    kernelPageDirectory = paging_new(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernelPageDirectory);
    enable_paging();

    // Register 
    isr80h_register_commands();
    
    // Initialize all keyboard
    keyboard_init();

    // Load first process
        struct Process * process = 0;
        int res = process_load_switch("0:/blank.elf",&process);
        
        struct CommandArgument argument;
        
        strcpy(argument.argument, "P1");
        argument.next = 0x00; 
        process_inject_arguments(process, &argument);
    
    // Load second process
        res = process_load_switch("0:/blank.elf",&process);
        
        strcpy(argument.argument, "P2");
        argument.next = 0x00; 
        process_inject_arguments(process, &argument);


    if(res != BIMBLEOS_ALL_OK){
        panic("Failed to load process");
    }

    
    task_run_first_ever_task(); 


 }