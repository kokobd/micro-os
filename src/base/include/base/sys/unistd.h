#pragma once

#include <stdint.h>
#include <stddef.h>

int fork();

int replaceMe(void *image, size_t size, void *entryPoint);

void exit();

void *sbrk(ptrdiff_t diff);

void releasePrivilege();

enum ListenToIRQError {
    IRQ_NO_ERROR = 0,
    IRQ_PERM_DENIED = 1,
    IRQ_INVALID_ARGS = 2
};

enum ListenToIRQError listenToIRQ(uint32_t irqNum, int msgBoxId);

int mapPhysicalMemory(uintptr_t page, uintptr_t frame);
