#include <process/Process.h>
#include <process/scheduler.h>
#include <process/syscall.h>
#include <cpu/RegState.h>

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

enum MsgBoxError closeMsgBox(int id) {
    struct Process    *current = currentProcess();
    struct MessageBox *msgBox  = Process_msgBox(current, id);
    if (msgBox == NULL) {
        return MBE_INVALID_ID;
    }
    MB_initInvalid(msgBox);
    return MBE_NO_ERROR;
}

enum MsgBoxError moveMsgBox(int id, void *newLocation) {
    struct Process    *current = currentProcess();
    struct MessageBox *msgBox  = Process_msgBox(current, id);
    if (msgBox == NULL) {
        return MBE_INVALID_ID;
    }

    if (!MB_isValid(msgBox)) {
        return MBE_NOT_INITIALIZED;
    }

    if (!Process_ownMemory(current, (uintptr_t) newLocation, MB_sizeInBytes(msgBox))) {
        return MBE_INVALID_ARGS;
    }

    MB_moveData(msgBox, newLocation);
    return MBE_NO_ERROR;
}

enum SendError sendPacket(const struct Packet *packet) {
    struct Process *current = currentProcess();

    if (!Process_ownMemory(current, (uintptr_t) packet, sizeof(struct Packet))) {
        return SE_INVALID_ARGS;
    }

    struct Process *destProcess = getProcessByID((ProcID) packet->pid);
    if (destProcess == NULL) {
        return SE_DEST_NOT_REACHABLE;
    }
    struct MessageBox *msgBox = Process_msgBox(destProcess, packet->msgBoxId);
    if (msgBox == NULL || !MB_isValid(msgBox)) {
        return SE_DEST_NOT_REACHABLE;
    }

    if (MB_isFull(msgBox)) {
        return SE_DEST_TOO_BUSY;
    }

    if (!Process_ownMemory(current, (uintptr_t) packet->message, MB_msgSize(msgBox))) {
        return SE_INVALID_ARGS;
    }

    MB_push(msgBox, packet->message);
    if (destProcess->status.type == PS_WAITING) {
        if (destProcess->status.boxId == packet->msgBoxId) {
            void *buffer = (void *) destProcess->regState.ebx;
            MB_pop(msgBox, buffer);
        }
    } else if (destProcess->status.type == PS_WAITING_ANY) {
        void *buffer = (void *) destProcess->regState.eax;
        MB_pop(msgBox, buffer);
    }
    destProcess->status.type = PS_READY;

    return SE_NO_ERROR;
}

ProcID fork() {
    ProcID childID = forkProcess(currentPID());
    getProcessByID(childID)->regState.eax = 0;
    return childID;
}
