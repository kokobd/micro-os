#include <process/scheduler.h>
#include <process/Process.h>
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

void schedule(RegState *regState) {
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
    restoreProcess(getByID(scheduler.current), regState);
}

struct Process *getByID(ProcID id) {
    return scheduler.processes + id - PROC_ID_MIN;
}

static void restoreProcess(struct Process *process, RegState *regState) {
    *regState = process->regState;
    Process_applyPageTable(process);
}

struct Process *currentProcess() {
    return getByID(scheduler.current);
}

void restoreCurrentProcess(RegState *regState) {
    restoreProcess(currentProcess(), regState);
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
