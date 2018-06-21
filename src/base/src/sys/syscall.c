#include <stdint.h>
#include "syscall.h"

uint32_t syscall(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    register uint32_t eax asm("eax");
    register uint32_t ebx asm("ebx");
    register uint32_t ecx asm("ecx");
    register uint32_t edx asm("edx");
    register uint32_t esi asm("esi");
    esi = id;
    eax = arg1;
    ebx = arg2;
    ecx = arg3;
    edx = arg4;
    asm (
    "int 0x30"
    : : : "eax"
    );
    return eax;
}
