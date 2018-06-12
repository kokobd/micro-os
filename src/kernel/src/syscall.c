#include <cpu/interrupt.h>

void cpu_syscallHandler(RegState *regState) {
    asm volatile ("nop");
    // TODO
}
