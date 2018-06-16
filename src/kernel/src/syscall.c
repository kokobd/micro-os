#include <cpu/interrupt.h>
#include <cpu/RegState.h>
#include <process/syscall.h>
#include <process/scheduler.h>

void cpu_syscallHandler(RegState *regState) {
    uint32_t       ret   = 0;
    struct Process *prev = currentProcess();

    switch (regState->esi) {
        case 0:
            ret = (uint32_t) maxMsgBoxId();
            break;
        case 1:
            ret = initMsgBox((int) regState->eax, (const struct MsgBoxInfo *) regState->ebx);
            break;
        case 2:
            ret = recvMsgFrom((int) regState->eax, (void *) regState->ebx);
            break;
        case 3:
            ret = recvAnyMsg((void *) regState->eax);
            break;
        default:
            break;
    }
    prev->regState.eax = ret;
    restoreCurrentProcess(regState);
}

