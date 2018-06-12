#include "int.h"
#include <cpu/interrupt.h>

void cpu_intHandler(uint32_t interrupt, RegState *regState) {
    if (interrupt >= 0x0 && interrupt <= 0x1F) {
        if (interrupt <= 0x14 && interrupt != 0x9 && interrupt != 0xF
            || interrupt == 0x1E) {
            cpu_excHandler((enum cpu_Exception) interrupt, regState);
        }
    } else if (interrupt >= 0x20 && interrupt <= 0x2F) {
        cpu_extIntHandler(interrupt - 0x20, regState);
    } else if (interrupt == 0x30) {
        cpu_syscallHandler(regState);
    }
}
