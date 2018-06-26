#include <process/Process.h>
#include <process/scheduler.h>
#include <process/syscall.h>
#include <cpu/RegState.h>
#include <ram/constants.h>
#include <process/IRQManager.h>
#include <core/utility.h>

int maxMsgBoxId() {
    return MSGBOX_LIMIT - 1;
}

enum MsgBoxError initMsgBox(int id, const struct MsgBoxInfo *info) {
    struct Process *current = currentProcess();
    struct MessageBox *msgBox = Process_msgBox(current, id);
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
    struct Process *current = currentProcess();
    struct MessageBox *msgBox = Process_msgBox(current, id);
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

    size_t maxBufferSize = 0;
    bool hasMsgBoxInitialized = false;
    uint8_t msgBoxID = MSGBOX_LIMIT;

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
    struct Process *current = currentProcess();
    struct MessageBox *msgBox = Process_msgBox(current, id);
    if (msgBox == NULL) {
        return MBE_INVALID_ID;
    }
    MB_initInvalid(msgBox);
    return MBE_NO_ERROR;
}

enum MsgBoxError moveMsgBox(int id, void *newLocation) {
    struct Process *current = currentProcess();
    struct MessageBox *msgBox = Process_msgBox(current, id);
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

    const ProcID destID = (const ProcID) packet->pid;
    struct Process *destProcess = getProcessByID(destID);
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

    sendMessageTo(destID, (uint8_t) packet->msgBoxId, packet->message);

    return SE_NO_ERROR;
}

ProcID fork() {
    ProcID childID = forkProcess(currentPID());
    getProcessByID(childID)->regState.eax = 0;
    return childID;
}

int replaceMe(void *image, size_t size, void *entryPoint) {
    const uintptr_t image_ = (uintptr_t) image;
    const uintptr_t entryPoint_ = (uintptr_t) entryPoint;

    // not aligned to page boundary?
    if ((image_ >> PAGE_SHIFT) << PAGE_SHIFT != image_) {
        return 1;
    }

    // entry point not in range?
    if (entryPoint >= image + size) {
        return 1;
    }

    Process_replaceMe(currentProcess(), image_, size, entryPoint_);

    return 0;
}

void exit() {
    killCurrentProcess();
}

void *sbrk(ptrdiff_t diff) {
    return (void *) Process_sbrk(currentProcess(), diff);
}

void releasePrivilege() {
    Process_setIsRoot(currentProcess(), false);
}

enum ListenToIRQError listenToIRQ(uint32_t irqNum, int msgBoxId) {
    struct Process *current = currentProcess();
    if (!Process_getIsRoot(current)) {
        return IRQ_PERM_DENIED;
    }

    struct MessageBox *mb = Process_msgBox(current, msgBoxId);
    if (mb == NULL || MB_msgSize(mb) != sizeof(uint32_t)) {
        return IRQ_INVALID_ARGS;
    }

    IRQManager_register(getIRQManager(), irqNum, currentPID(), (uint8_t) msgBoxId);
    return IRQ_NO_ERROR;
}

int mapPhysicalMemory(uintptr_t page, uintptr_t frame) {
    if (!core_isAligned(page, PAGE_SHIFT) || !core_isAligned(frame, PAGE_SHIFT)) {
        return 1;
    }

    if (!Process_mapPhysicalMemory(currentProcess(), page, frame)) {
        return 1;
    }

    return 0;
}
