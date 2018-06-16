#include <cpu/interrupt.h>
#include <process/syscall.h>
#include <cpu/RegState.h>

void cpu_syscallHandler(RegState *regState) {
    uint32_t ret = 0;
    switch (regState->esi) {
        case 0:
            ret = (uint32_t) maxMsgBoxId();
            break;
        case 1:
            ret = initMsgBox((int) regState->eax, (const struct MsgBoxInfo *) regState->ebx);
            break;
        default:
            break;
    }
    regState->eax = ret;
}

