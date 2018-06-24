#pragma once

#include <cpu/interrupt.h>
#include "Process.h"

struct IRQManager {
    ProcID irqToProcID[IRQ_COUNT];
    uint8_t irqToMsgBoxID[IRQ_COUNT];
};

void IRQManager_init(struct IRQManager *obj);

void IRQManager_register(struct IRQManager *obj, uint32_t irqNum, ProcID pid, uint8_t msgBoxId);

void IRQManager_handleIRQ(const struct IRQManager *obj, uint32_t irqNum);
