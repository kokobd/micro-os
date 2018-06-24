#include <cpu/interrupt.h>
#include <process/scheduler.h>
#include <process/IRQManager.h>

void cpu_extIntHandler(uint32_t irq_num) {
    switch (irq_num) {
        case 0:
            dispatch();
            break;
        default:
            IRQManager_handleIRQ(getIRQManager_const(), irq_num);
            break;
    }
}
