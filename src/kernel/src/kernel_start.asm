BITS 32

extern main

global kernel_start

[section .text]
kernel_start:
    mov ebp, 0x7FFFF
    mov esp, ebp

    push ebx
    push eax
    call main

    cli
    hlt