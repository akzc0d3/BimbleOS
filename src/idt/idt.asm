section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler
extern interrupt_handler



global idt_load
global enable_interrupts
global disable_interrupts
global isr80h_wrapper
global interrupt_pointer_table

idt_load:
    push ebp
    mov ebp , esp 
    mov ebx, [ebp+8]
    lidt [ebx]
    pop ebp
    ret

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

 




%macro interrupt 1
    global int%1
    int%1:
    ; INTERRUPT FRAME START
        ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
            ; uint32_t ip
            ; uint32_t cs;
            ; uint32_t flags
            ; uint32_t sp;
            ; uint32_t ss;
    ; Pushes EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI
        pushad

    ; INTERRUPT FRAME END
        push esp
        push dword %1
        call interrupt_handler
        add esp, 8
        popad
        iret
%endmacro

%assign i 0
%rep 256
    interrupt i
%assign i i+1
%endrep



isr80h_wrapper:
    ; INTERRUPT FRAME START
        ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
            ; uint32_t ip
            ; uint32_t cs;
            ; uint32_t flags
            ; uint32_t sp;
            ; uint32_t ss;
    ; Pushes EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI
    pushad      
    
    ; INTERRUPT FRAME END

    ; Push the stack pointer so that we are pointing to the interrupt frame. This is used to access interrupt frame
    push esp    

    ; EAX holds our command lets push it to the stack for isr80h_handler
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax     ; We used "tmp_res" here to avoid accidental overwriting eax further
    add esp, 8

    ; Restore general purpose registers for user land
    popad
    mov eax, [tmp_res]
    iretd






section .data
tmp_res: dd 0   ; Inside here is stored the return result from isr80h_handler



%macro interrupt_array_entry 1
    dd int%1
%endmacro

interrupt_pointer_table:
%assign i 0
%rep 256
    interrupt_array_entry i
%assign i i+1
%endrep