#include <process/scheduler.h>
#include <process/Process.h>
#include "ProcessQueue.h"

struct Scheduler {
    struct Process processes[PROC_ID_COUNT];
    ProcID current;
    struct ProcessQueue ready;
};

static struct Scheduler scheduler;

static struct Process *getByID(ProcID id);

static void restoreProcess(struct Process *process, RegState *regState);

void schedule(RegState *regState) {
    if (scheduler.current == PROC_ID_NULL) {
        if (PQ_isEmpty(&scheduler.ready)) {
            return;
        } else {
            scheduler.current = PQ_pop(&scheduler.ready);
        }
    } else {
        ProcID prev = scheduler.current;
        scheduler.current = PQ_pop(&scheduler.ready);
        PQ_push(&scheduler.ready, prev);
    }
    restoreProcess(getByID(scheduler.current), regState);
}

struct Process *getByID(ProcID id) {
    return scheduler.processes + id - PROC_ID_MIN;
}

static void restoreProcess(struct Process *process, RegState *regState) {
    *regState = process->regState;
    Proc_switchPageTable(process);
}
