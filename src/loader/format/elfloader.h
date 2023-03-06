#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "elf.h"


/**
 * @brief Represents an entire ELF file loaded in memory
 * 
 */
struct Elf_file
{
    char filename[BIMBLEOS_MAX_PATH];

    int in_memory_size;

    /**
     * The physical memory address that this elf file is loaded at
     */
    void* elf_memory;

    /**
     * The virtual base address of this binary
     */
    void* virtual_base_address;

    /**
     * The ending virtual address
     */
    void* virtual_end_address;

    /**
     * The physical base address of this binary
     */
    void* physical_base_address;

    /**
     * The physical end address of this bunary
     */
    void* physical_end_address;
    

};

int elf_load(const char* filename, struct Elf_file** file_out);
void elf_close(struct Elf_file* file);
void* elf_virtual_base(struct Elf_file* file);
void* elf_virtual_end(struct Elf_file* file);
void* elf_phys_base(struct Elf_file* file);
void* elf_phys_end(struct Elf_file* file);

struct Elf_header* elf_header(struct Elf_file* file);
struct Elf32_shdr* elf_sheader(struct Elf_header* header);
void* elf_memory(struct Elf_file* file);
struct Elf32_phdr* elf_pheader(struct Elf_header* header);
struct Elf32_phdr* elf_program_header(struct Elf_header* header, int index);
struct Elf32_shdr* elf_section(struct Elf_header* header, int index);
void* elf_phdr_phys_address(struct Elf_file* file, struct Elf32_phdr* phdr);

#endif