global crt_call_main

extern main

[SECTION .text]
crt_call_main:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    push ebx ; push the second param
    push eax ; push the first param
    call main
    ret

