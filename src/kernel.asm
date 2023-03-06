[BITS 32]


global _start
global kernel_registers
 
extern kernel_main

CODE_SEG equ 0x8
DATA_SEG equ 0x10

_start:
    ;Initialize segment selector (will hold the offset of data segement in GDT)
        mov ax, DATA_SEG
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov fs, ax
        mov gs, ax
        mov ebp, 0x00200000
        mov esp, ebp


    ;Enable A20 line
        in al, 0x92
        or al, 2    
        out 0x92, al

    ;Remap Master PIC
        mov al,00010001b    ; Put PIC in initialization mode
        out 0x20, al

        mov al, 0x20        ; Map PIC to start with  0x20 i.e. IRQ 0 will be INT 0x20
        out 0x21, al     

        mov al, 00000001b   ; Put PIC in x86 mode
        out 0x21, al

    call kernel_main
    jmp $
 
   
kernel_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret  
     

times 512-($-$$)  db 0