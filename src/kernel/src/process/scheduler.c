#include <process/scheduler.h>
#include "ProcessQueue.h"
#include <process/IRQManager.h>
#include <process/ELFImage.h>

struct Scheduler {
    struct Process processes[PROC_ID_COUNT];
    ProcID current;
    struct ProcessQueue ready;
    struct IRQManager irqManager;
};

static struct Scheduler scheduler;

static struct Process *getByID(ProcID id);

static void restoreProcess(struct Process *process, RegState *regState);

/**
 * Find next ready process and set it as
 * the current process. This function doesn't
 * copy register state or apply page table.
 */
static void executeNextProcess() {
    if (PQ_isEmpty(&scheduler.ready)) {
        // What should we do?
    } else {
        scheduler.current = PQ_pop(&scheduler.ready);
    }
}

static ProcID newPID() {
    for (ProcID id = PROC_ID_MIN; id <= PROC_ID_MAX; ++id) {
        struct Process *process = getByID(id);
        if (process->status.type == PS_INVALID) {
            return id;
        }
    }
    return PROC_ID_NULL;
}

void dispatch() {
    if (scheduler.current == PROC_ID_NULL) {
        if (PQ_isEmpty(&scheduler.ready)) {
            return;
        } else {
            scheduler.current = PQ_pop(&scheduler.ready);
        }
    } else {
        ProcID prev = scheduler.current;
        executeNextProcess();
        PQ_push(&scheduler.ready, prev);
    }
}

static struct Process *getByID(ProcID id) {
    return scheduler.processes + id - PROC_ID_MIN;
}

static void restoreProcess(struct Process *process, RegState *regState) {
    *regState = process->regState;
    Process_applyPageTable(process);
}

struct Process *currentProcess() {
    if (scheduler.current == PROC_ID_NULL) {
        return NULL;
    } else {
        return getByID(scheduler.current);
    }
}

void restoreCurrentProcess(RegState *regState) {
    if (currentProcess())
        restoreProcess(currentProcess(), regState);
    else {
        asm volatile(
        "sti\n"
        "hlt"
        );
    }
}

void wait(uint8_t msgBoxID) {
    struct Process *current = currentProcess();

    if (msgBoxID != MSGBOX_LIMIT) {
        current->status.type = PS_WAITING;
        current->status.boxId = msgBoxID;
    } else {
        current->status.type = PS_WAITING_ANY;
    }

    executeNextProcess();
}

void notify(ProcID procID) {
    struct Process *process = getByID(procID);
    process->status.type = PS_READY;
    PQ_push(&scheduler.ready, procID);
}

struct Process *getProcessByID(ProcID id) {
    if (id <= PROC_ID_MAX && id >= PROC_ID_MIN) {
        return getByID(id);
    } else {
        return NULL;
    }
}

ProcID forkProcess(ProcID parentID) {
    ProcID childID = newPID();
    if (childID == PROC_ID_NULL) {
        return childID;
    }
    struct Process *parent = getByID(parentID);
    struct Process *child = getByID(childID);
    Process_fork(parent, child);
    return childID;
}

ProcID currentPID() {
    return scheduler.current;
}

void initScheduler() {
    scheduler.current = PROC_ID_NULL;
    PQ_init(&scheduler.ready);

    for (ProcID id = PROC_ID_MIN; id <= PROC_ID_MAX; ++id) {
        getByID(id)->status.type = PS_INVALID;
    }

    IRQManager_init(&scheduler.irqManager);
}

void saveRegState(const RegState *regState) {
    if (currentProcess())
        currentProcess()->regState = *regState;
}

void killCurrentProcess() {
    ProcID parentID = currentProcess()->parent;
    struct Process *parent = getProcessByID(parentID);
    if (parent != NULL) {
        struct MessageBox *mb = Process_msgBox(parent, 0);
        if (mb != NULL && MB_isValid(mb) && MB_msgSize(mb) == 8) {
            uint32_t msg[2];
            msg[1] = currentPID();
            sendMessageTo(parentID, 0, msg);
        }
    }

    Process_exit(currentProcess());
    executeNextProcess();
}

void sendMessageTo(ProcID pid, uint8_t msgBoxId, void *message) {
    struct Process *process = getProcessByID(pid);
    if (process != NULL) {
        struct MessageBox *mb = Process_msgBox(process, msgBoxId);
        if (mb != NULL && MB_isValid(mb) && MB_msgSize(mb) > 4 && !MB_isFull(mb)) {
            ProcID pid_src = currentPID();
            if (pid_src == PROC_ID_NULL)
                pid_src = PROC_ID_KERNEL;
            *(uint8_t *) message = pid_src;
            *((uint8_t *) message + 1) = pid_src == getProcessByID(pid)->parent ?
                                         (uint8_t) 1 : (uint8_t) 0;

            MB_push(mb, message);
            void *buffer = NULL;
            if (process->status.type == PS_WAITING) {
                if (process->status.boxId == msgBoxId) {
                    buffer = (void *) process->regState.ebx;
                }
            } else if (process->status.type == PS_WAITING_ANY) {
                buffer = (void *) process->regState.eax;
            }
            if (buffer) {
                MB_pop(mb, buffer);
                notify(pid);
            }
        }
    }
}

const struct IRQManager *getIRQManager_const() {
    return &scheduler.irqManager;
}

struct IRQManager *getIRQManager() {
    return &scheduler.irqManager;
}

ProcID processToPID(const struct Process *process) {
    if (process == NULL) {
        return PROC_ID_NULL;
    }
    return (ProcID) (process - scheduler.processes + PROC_ID_MIN);
}

void addInitialProcess(uintptr_t location) {
    struct ELFImage image;
    ELFImage_init(&image, location);
    uintptr_t entryPoint = ELFImage_getEntryPoint(&image);
    size_t memSize = ELFImage_requestedSize(&image);
    size_t codeSize = ELFImage_codeSize(&image);

    ProcID pid = newPID();
    Process_createWithImage(getByID(pid), location, codeSize, memSize, entryPoint);
    PQ_push(&scheduler.ready, pid);
}
