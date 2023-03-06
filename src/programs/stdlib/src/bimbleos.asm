[BITS 32]

section .asm

global print:function
global bimbleos_getkey:function
global bimbleos_malloc:function
global bimbleos_free:function
global bimbleos_putchar:function
global bimbleos_process_load_start:function
global bimbleos_process_get_arguments:function
global bimbleos_system:function
global bimbleos_exit:function


; void print(const char*)
print:
    push ebp
    mov ebp, esp
    push dword[ebp + 8]
    mov eax, 1      ; print command
    int 0x80
    add esp, 4
    pop ebp
    ret



; int bimbleos_getkey()
bimbleos_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2      ; getkey command
    int 0x80
    pop ebp
    ret

; void* bimbleos_malloc(size_t)
bimbleos_malloc:
    push ebp
    mov ebp, esp
    push dword[ebp + 8]
    mov eax, 4     ; malloc command
    int 0x80
    add esp, 4
    pop ebp
    ret



; void bimbleos_free(void*)
bimbleos_free:
    push ebp
    mov ebp, esp
    push dword[ebp + 8]
    mov eax, 5     ; free command
    int 0x80
    add esp, 4
    pop ebp
    ret

;void putchar(char c)
bimbleos_putchar:
    push ebp
    mov ebp, esp
    push dword [ebp + 8]
    mov eax, 3       ; putchar command
    int 0x80
    add esp, 4
    pop ebp
    ret


;void bimbleos_process_load_start(const char* filename)
bimbleos_process_load_start:
    push ebp
    mov ebp, esp
    push dword [ebp + 8]
    mov eax, 6          ; process_load command
    int 0x80
    add esp, 4
    pop ebp
    ret


; void bimbleos_process_get_arguments(struct ProcessArguments* arguments)
bimbleos_process_get_arguments:
    push ebp
    mov ebp, esp
    push dword[ebp+8]   ; Variable arguments
    mov eax, 8          ; Gets the process arguments command
    int 0x80
    add esp, 4
    pop ebp
    ret

; int bimbleos_system(struct CommandArgument* arguments)
bimbleos_system:
    push ebp
    mov ebp, esp
    push dword[ebp+8]   ; Variable "arguments"
    mov eax, 7          ; runs a system command 
    int 0x80
    add esp, 4
    pop ebp
    ret

; void bimbleos_exit()
bimbleos_exit:
    push ebp
    mov ebp, esp
    mov eax, 9          ; Command 9 process exit
    int 0x80
    pop ebp
    ret