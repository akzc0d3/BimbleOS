#ifndef PROCESS_H
#define PROCESS_H

#include "task.h"
#include "config.h"
#include "loader/format/elfloader.h"
#include <stdint.h>
#include <stdbool.h>

#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;


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
struct ProcessAllocation
{
    void* ptr;
    size_t size;
};

struct Process
{
    // The process id
    uint16_t id;

    char filename[BIMBLEOS_MAX_PATH];

    // The main process task
    struct Task *task;

    // The memory (malloc) allocations of the process
    struct ProcessAllocation allocations[BIMBLEOS_MAX_PROGRAM_ALLOCATIONS];


    PROCESS_FILETYPE filetype;
    union 
    {
        // The physical pointer to the process's memory i.e. location where process is loaded into memeory (code + data).
        void *ptr;
        struct Elf_file* elf_file;
    };
    

    // The physical pointer to the stack memory
    void *stack;

    // The size of the data pointed to by "ptr"
    uint32_t size;


    struct KeyboardBuffer
    {
        char buffer[BIMBLEOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    }keyboard;


     // The arguments of the process.
    struct ProcessArguments arguments;
};


int process_load_switch(const char* filename, struct Process** process);
struct Process* process_current();
struct Process* process_get(int process_id); 
void* process_malloc(struct Process* process, size_t size);
void process_free(struct Process* process, void* ptr);
void process_get_arguments(struct Process* process, int* argc, char*** argv);
int process_inject_arguments(struct Process* process, struct CommandArgument* root_argument); 
int process_terminate(struct Process* process);

#endif