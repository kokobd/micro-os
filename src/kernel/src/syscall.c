#include <cpu/interrupt.h>

void cpu_syscallHandler(cpu_RegState *regState) {
    asm volatile ("nop");
    // TODO
}
