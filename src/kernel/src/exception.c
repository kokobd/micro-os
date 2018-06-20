#include <cpu/interrupt.h>
#include <process/scheduler.h>
#include <cpu/RegState.h>

static void onPageFault() {
    const uint32_t pfErrCode = currentProcess()->regState.errorCode;

    bool           present  = (bool) (pfErrCode & 0b1u);
    bool           write    = (bool) (pfErrCode & 0b10u);
    bool           user     = (bool) (pfErrCode & 0b100u);
    bool           insFetch = (bool) (pfErrCode & 0b1000u);

    uintptr_t vaddr;
    asm volatile (
    "mov %0, cr2"
    : "=r" (vaddr)
    );

    // 1. 先检查当前进程正在访问合法内存区域
    // 2. 判断是不是COW引起的
}

void cpu_excHandler(enum cpu_Exception exception) {
    switch (exception) {
        case CPU_EXC_PF:
            onPageFault();
            break;
        default:
            break;
    }
}
