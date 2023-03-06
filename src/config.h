#ifndef CONFIG_H
#define CONFIG_H

#define KERNAL_CODE_SELECTOR                                0x08
#define KERNAL_DATA_SELECTOR                                0x10

#define BIMBLEOS_TOTAL_INTERRUPTS                           256


#define BIMBLEOS_HEAP_SIZE_BYTES                            104857600   // 100 MB
#define BIMBLEOS_HEAP_BLOCK_SIZE                            4096

#define BIMBLEOS_HEAP_ADDRESS                               0x01000000
#define BIMBLEOS_HEAP_TABLE_ADDRESS                         0x00007E00
#define BIMBLEOS_SECTOR_SIZE                                512
#define BIMBLEOS_MAX_PATH                                   108
#define BIMBLEOS_MAX_FILESYSTEMS                            12
#define BIMBLEOS_MAX_FILE_DESCRIPTORS                       512

    
#define BIMBLEOS_TOTAL_GDT_SEGMENT                          6
#define BIMBLEOS_PROGRAM_VIRTUAL_ADDRESS                    0x400000
#define BIMBLEOS_USER_PROGRAM_STACK_SIZE                    1024 * 16
#define BIMBLEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START        0x3FF000
#define BIMBLEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END          BIMBLEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START - BIMBLEOS_USER_PROGRAM_STACK_SIZE

#define USER_CODE_SEGMENT                                   0x1B        // Offset of code segment in GDT: Includes ring level (of userland) bits too         
#define USER_DATA_SEGMENT                                   0x23        // Offset of data segment in GDT: Includes ring level (of userland) bits too         

#define BIMBLEOS_MAX_PROGRAM_ALLOCATIONS                    1024
#define BIMBLEOS_MAX_PROCESSES                              12

#define BIMBLEOS_MAX_ISR80H_COMMANDS                         1024
#define BIMBLEOS_KEYBOARD_BUFFER_SIZE                        1024
#endif