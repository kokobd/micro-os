#pragma once

#include <cpu/RegState.h>
#include <process/MessageBox.h>

#define MSGBOX_LIMIT 8u

#define PROC_ID_MIN 1u

#define PROC_ID_MAX 254u

#define PROC_ID_NULL 0u

#define PROC_ID_KERNEL 255u

#define PROC_ID_COUNT (PROC_ID_MAX - PROC_ID_MIN + 1)

typedef uint8_t ProcID;

enum ProcStatusType {
    PS_READY, PS_WAITING, PS_WAITING_ANY, PS_INVALID
};

struct ProcStatus {
    enum ProcStatusType type;
    uint8_t boxId;
};

/**
 * Each process has a PCB. When the process is running, the
 * pcb will be located at a specific virtual address.
 *
 * PCB should contain:
 *   - program break
 *   - register state
 *   - array of pointers to message box.
 *   - ready? waiting for a specific message box? waiting for any message?
 */
struct Process {
    uintptr_t pageTableRoot;
    RegState regState;
    uintptr_t programBreak;
    struct MessageBox msgBoxes[MSGBOX_LIMIT];
    struct ProcStatus status;
    bool isRoot;
};

void Process_applyPageTable(struct Process *process);

/**
 * Create a process with a memory image.
 */
void Process_createWithImage(struct Process *process, uintptr_t imagePAddr,
                             size_t imageSize, size_t requestedSize,
                             uintptr_t entryPoint);

/**
 * Copy process src to dest. COW strategy will be employed.
 * 'child' is assumed to be uninitialized. The child process
 * will have the same register state, a copy of the page table,
 * the same program break, invalid message boxes, and the same
 * ProcStatus
 * @param parent pointer to the parent process
 * @param child pointer to the child process
 */
void Process_fork(struct Process *parent, struct Process *child);

/**
 * Destructs a process and switch to Identity PageTable.
 * After calling this function, the pointer should be considered as invalid.
 * @param process pointer to the process.
 */
void Process_exit(struct Process *process);

struct MessageBox *Process_msgBox(struct Process *process, int msgBoxID);

bool Process_ownMemory(struct Process *process, uintptr_t begin, size_t size);

inline static bool Process_isValid(struct Process *process) {
    return process->status.type != PS_INVALID;
}

void Process_replaceMe(struct Process *process, uintptr_t image, size_t size, uintptr_t entryPoint);

uintptr_t Process_sbrk(struct Process *process, ptrdiff_t diff);
