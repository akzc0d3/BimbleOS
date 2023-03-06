[BITS 32]

section .asm

global restore_general_purpose_registers
global task_return
global user_registers



; IRET will 

; void task_return(struct Registers* regs);
task_return:
    mov ebp, esp
    ; PUSH THE DATA SEGMENT (SS WILL BE FINE)
    ; PUSH THE STACK ADDRESS
    ; PUSH THE FLAGS
    ; PUSH THE CODE SEGMENT
    ; PUSH IP

    ; Let's access the structure passed to us
    mov ebx, [ebp+4]
    
    ; Data Selector
    push dword [ebx+44]     
    
    ; Stack pointer kernel.c:135
    push dword [ebx+40]     

    ; Flags
    pushf
    pop eax
    or eax, 0x200           ; Set Interrupt enable flag
    push eax

    ; Code Segment
    push dword [ebx+32]

    ; IP
    push dword [ebx+28]

    ; Setup some segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebp+4]
    call restore_general_purpose_registers
    add esp, 4

    ; Let's leave kernel land and execute in user land!
    iretd

; void restore_general_purpose_registers(struct registers* regs);
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]
    pop ebp 
    ret


; void user_registers()
user_registers:
    mov ax, 0x23    ; DS will point to user data segment 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret