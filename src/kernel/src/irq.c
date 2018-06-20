#include <cpu/interrupt.h>
#include <process/scheduler.h>

void cpu_extIntHandler(uint32_t irq_num) {
    switch (irq_num) {
        case 0:
            dispatch();
            break;
        default:
            break;
    }
}
