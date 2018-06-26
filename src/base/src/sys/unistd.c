#include <base/sys/unistd.h>
#include "syscall.h"

int fork() {
    return (int) syscall(7, 0, 0, 0, 0);
}

int replaceMe(void *image, size_t size, void *entryPoint) {
    return (int) syscall(8, (uint32_t) image, size, (uint32_t) entryPoint, 0);
}

void exit() {
    syscall(9, 0, 0, 0, 0);
}

void *sbrk(ptrdiff_t diff) {
    return (void *) syscall(10, (uint32_t) diff, 0, 0, 0);
}

void releasePrivilege() {
    syscall(11, 0, 0, 0, 0);
}

enum ListenToIRQError listenToIRQ(uint32_t irqNum, int msgBoxId) {
    return (enum ListenToIRQError) syscall(12, irqNum, (uint32_t) msgBoxId, 0, 0);
}

int mapPhysicalMemory(uintptr_t page, uintptr_t frame) {
    return (int) syscall(13, page, frame, 0, 0);
}
