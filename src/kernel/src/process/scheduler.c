#include <process/scheduler.h>
#include <process/Process.h>
#include <cpu/RegState.h>
#include "ProcessQueue.h"

struct Scheduler {
    struct Process      processes[PROC_ID_COUNT];
    ProcID              current;
    struct ProcessQueue ready;
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

struct Process *getByID(ProcID id) {
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
        current->status.type  = PS_WAITING;
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
    struct Process *child  = getByID(childID);
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
}

void saveRegState(const RegState *regState) {
    if (currentProcess())
        currentProcess()->regState = *regState;
}
