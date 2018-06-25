#include <cpu/interrupt.h>
#include <process/scheduler.h>
#include "ram/pageFaultHandler.h"

void cpu_excHandler(enum cpu_Exception exception) {
    switch (exception) {
        case CPU_EXC_PF:
            onPageFault();
            break;
        default:
            if (currentProcess())
                killCurrentProcess();
            break;
    }
}

