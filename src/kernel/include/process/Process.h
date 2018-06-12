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
    PS_READY, PS_WAITING, PS_WAITING_ANY
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
};

void Proc_switchPageTable(struct Process *process);
