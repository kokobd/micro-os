#include <base/sys/ipc.h>
#include "syscall.h"

int maxMsgBoxId() {
    return (int) syscall(0, 0, 0, 0, 0);
}

enum MsgBoxError initMsgBox(int id, const struct MsgBoxInfo *info) {
    return (enum MsgBoxError) syscall(1, (uint32_t) id, (uint32_t) info, 0, 0);
}

enum MsgBoxError recvMsgFrom(int id, struct Message *buffer) {
    return (enum MsgBoxError) syscall(2, (uint32_t) id, (uint32_t) buffer, 0, 0);
}

enum MsgBoxError recvAnyMsg(struct Message *buffer) {
    return (enum MsgBoxError) syscall(3, (uint32_t) buffer, 0, 0, 0);
}

enum MsgBoxError closeMsgBox(int id) {
    return (enum MsgBoxError) syscall(4, (uint32_t) id, 0, 0, 0);
}

enum MsgBoxError moveMsgBox(int id, void *newLocation) {
    return (enum MsgBoxError) syscall(5, (uint32_t) id, (uint32_t) newLocation, 0, 0);
}

enum SendError sendPacket(const struct Packet *packet) {
    return (enum SendError) syscall(6, (uint32_t) packet, 0, 0, 0);
}
