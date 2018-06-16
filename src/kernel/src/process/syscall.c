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

enum MsgBoxError recvMsgFrom(int id, void *buffer) {
    struct Process    *current = currentProcess();
    struct MessageBox *msgBox  = Process_msgBox(current, id);
    if (msgBox == NULL) {
        return MBE_INVALID_ID;
    }
    if (!MB_isValid(msgBox)) {
        return MBE_NOT_INITIALIZED;
    }
    if (!Process_ownMemory(current, (uintptr_t) buffer, msgBox->msgSize)) {
        return MBE_INVALID_ARGS;
    }

    if (MB_isEmpty(msgBox)) {
        wait((uint8_t) id);
    } else {
        MB_pop(msgBox, buffer);
    }
    return MBE_NO_ERROR;
}

enum MsgBoxError recvAnyMsg(void *buffer) {

    struct Process *current = currentProcess();

    size_t  maxBufferSize = 0;
    bool hasMsgBoxInitialized = false;
    uint8_t msgBoxID      = MSGBOX_LIMIT;

    for (uint8_t i = 0; i < MSGBOX_LIMIT; ++i) {
        struct MessageBox *msgBox = Process_msgBox(current, i);
        if (MB_isValid(msgBox)) {
            hasMsgBoxInitialized = true;
            if (msgBoxID == MSGBOX_LIMIT && !MB_isEmpty(msgBox)) {
                msgBoxID = i;
            }

            size_t bufferSize = msgBox->msgSize;
            if (bufferSize > maxBufferSize)
                maxBufferSize = bufferSize;
        }
    }
    if (!hasMsgBoxInitialized) {
        return MBE_NOT_INITIALIZED;
    } else if (!Process_ownMemory(current, (uintptr_t) buffer, maxBufferSize)) {
        return MBE_INVALID_ARGS;
    } else if (msgBoxID == MSGBOX_LIMIT) {
        wait(MSGBOX_LIMIT);
    } else {
        struct MessageBox *msgBox = Process_msgBox(current, msgBoxID);
        MB_pop(msgBox, buffer);
    }
    return MBE_NO_ERROR;
}
