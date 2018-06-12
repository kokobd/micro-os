#include <cpu/RegState.h>
#include <string.h>

void RegState_init(RegState *regState) {
    memset(regState, 0, sizeof(RegState));
    regState->ds = 0x23;
    regState->es = 0x23;
    regState->fs = 0x23;
    regState->gs = 0x23;
    regState->cs = 0x1B;
    regState->eflags = 0x3246;
    regState->ss = 0x23;
}
