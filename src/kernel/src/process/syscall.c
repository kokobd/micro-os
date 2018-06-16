#include <process/Process.h>
#include <process/scheduler.h>
#include <process/syscall.h>

int maxMsgBoxId() {
    return MSGBOX_LIMIT - 1;
}

enum MsgBoxError initMsgBox(int id, const struct MsgBoxInfo *info) {
    struct Process    *current = currentProcess();
    struct MessageBox *msgBox  = Process_msgBox(current, id);
    if (msgBox == NULL) {
        return MBE_INVALID_ID;
    }

    if (Process_ownMemory(current, (uintptr_t) info, sizeof(struct MsgBoxInfo))
        && Process_ownMemory(current, (uintptr_t) info->memory,
                             info->msgMaxCount * info->msgSize)) {
        MB_init(msgBox, info->msgSize, info->msgMaxCount, info->memory);
        return MBE_NO_ERROR;
    } else {
        return MBE_INVALID_ARGS;
    }
}
