#include <cpu/interrupt.h>
#include <process/scheduler.h>

void cpu_extIntHandler(uint32_t irq_num, RegState *regState) {
    switch (irq_num) {
        case 0:
            schedule(regState);
            break;
        default:
            break;
    }
}
