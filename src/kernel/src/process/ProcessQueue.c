#include "ProcessQueue.h"

inline static uint8_t PQ_next(const struct ProcessQueue *pq, uint8_t index) {
    ++index;
    if (index == PROC_ID_COUNT) {
        index = 0;
    }
    return index;
}

void PQ_init(struct ProcessQueue *pq) {
    pq->head = 0;
    pq->tail = PROC_ID_COUNT;
}

void PQ_push(struct ProcessQueue *pq, ProcID pid) {
    if (PQ_isEmpty(pq)) {
        pq->tail = pq->head;
    } else {
        pq->tail = PQ_next(pq, pq->tail);
    }
    pq->ids[pq->tail] = pid;
}

ProcID PQ_pop(struct ProcessQueue *pq) {
    ProcID ret = pq->ids[pq->head];
    pq->head = PQ_next(pq, pq->head);
    if (pq->head == pq->tail) {
        pq->tail = PROC_ID_COUNT;
    }
    return ret;
}
