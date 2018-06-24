#include <process/IRQManager.h>
#include <process/scheduler.h>

inline static bool isIRQNumValid(uint32_t irqNum) {
    return irqNum < IRQ_COUNT;
}

static void sendIRQMessage(uint32_t irqNum, ProcID pid, uint8_t msgBoxId);

void IRQManager_init(struct IRQManager *obj) {
    for (uint32_t irq = 0; irq < IRQ_COUNT; ++irq) {
        obj->irqToProcID[irq] = PROC_ID_NULL;
        obj->irqToMsgBoxID[irq] = MSGBOX_LIMIT;
    }
}

void IRQManager_register(struct IRQManager *obj, uint32_t irqNum, ProcID pid, uint8_t msgBoxId) {
    if (isIRQNumValid(irqNum)) {
        struct Process *process = getProcessByID(pid);
        if (process != NULL) {
            struct MessageBox *mb = Process_msgBox(process, msgBoxId);
            if (mb != NULL && mb->msgSize == sizeof(uint32_t)) {
                obj->irqToProcID[irqNum] = pid;
                obj->irqToMsgBoxID[irqNum] = msgBoxId;
            }
        }
    }
}

void IRQManager_handleIRQ(const struct IRQManager *obj, uint32_t irqNum) {
    if (isIRQNumValid(irqNum)) {
        ProcID pid = obj->irqToProcID[irqNum];
        uint8_t msgBoxId = obj->irqToMsgBoxID[irqNum];
        if (pid != PROC_ID_NULL && msgBoxId != MSGBOX_LIMIT)
            sendIRQMessage(irqNum, pid, msgBoxId);
    }
}

static void sendIRQMessage(uint32_t irqNum, ProcID pid, uint8_t msgBoxId) {
    sendMessageTo(pid, msgBoxId, &irqNum);
}
