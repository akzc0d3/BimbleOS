#ifndef BIMBLEOS_H
#define BIMBLEOS_H
#include <stddef.h>
#include <stdbool.h>


struct CommandArgument
{
    char argument[512];
    struct CommandArgument* next;
};
struct ProcessArguments
{
    int argc;
    char** argv;
};

void print(const char *);
void* bimbleos_malloc(size_t); 
void bimbleos_free(void*); 
void bimbleos_putchar(char); 
int bimbleos_getkey();   
int bimbleos_getkeyblock();
void bimbleos_terminal_realine(char *out, size_t max, bool echo);
void bimbleos_process_load_start(const char* filename);
struct CommandArgument* bimbleos_parse_command(const char* command, int max);
void bimbleos_process_get_arguments(struct ProcessArguments*);
int bimbleos_system(struct CommandArgument* arguments);
int bimbleos_system_run(const char* command);
void bimbleos_exit();

#endif