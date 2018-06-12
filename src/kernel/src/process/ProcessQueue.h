#pragma once

#include "../../include/process/Process.h"

struct ProcessQueue {
    ProcID ids[PROC_ID_COUNT];
    uint8_t head; // index of the first PID
    uint8_t tail; // index of the last PID
};

void PQ_init(struct ProcessQueue *pq);

inline static bool PQ_isEmpty(struct ProcessQueue *pq) {
    return pq->tail == PROC_ID_COUNT;
}

void PQ_push(struct ProcessQueue *pq, ProcID pid);

ProcID PQ_pop(struct ProcessQueue *pq);
