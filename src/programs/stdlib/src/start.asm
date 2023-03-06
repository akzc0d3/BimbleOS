[BITS 32]

section .asm


global _start
extern c_start
extern bimbleos_exit

_start:
    call c_start
    call bimbleos_exit
    ret