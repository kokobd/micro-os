#include <cpu/interrupt.h>
#include <cpu/RegState.h>
#include <process/syscall.h>
#include <process/scheduler.h>

void cpu_syscallHandler() {
    uint32_t ret = 0;
    struct Process *prev = currentProcess();
    const RegState *regState = &prev->regState;

    switch (prev->regState.esi) {
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
        case 4:
            ret = closeMsgBox((int) regState->eax);
            break;
        case 5:
            ret = moveMsgBox((int) regState->eax, (void *) regState->ebx);
            break;
        case 6:
            ret = sendPacket((const struct Packet *) regState->eax);
            break;
        case 7:
            ret = fork();
            break;
        case 8:
            ret = (uint32_t) replaceMe((void *) regState->eax, regState->ebx, (void *) regState->ecx);
            break;
        case 9:
            exit();
            break;
        case 10:
            ret = (uint32_t) sbrk(regState->eax);
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            releasePriviledge();
            break;
        default:
            break;
    }
    prev->regState.eax = ret;
}

