#include <cpu/RegState.h>
#include "Process.h"

void initScheduler();

void addInitialProcess(uintptr_t location);

void dispatch();

struct Process *currentProcess();

ProcID currentPID();

void restoreCurrentProcess(RegState *regState);

void saveRegState(const RegState *regState);

/**
 * Blocks current process. The process will
 * wait until a message is sent to the message box
 * with given id.
 * @param msgBoxID ID of the message box to wait.
 */
void wait(uint8_t msgBoxID);

/**
 * Notify a specific process. (The process may not execute immediately,
 * it will be pushed to the ready queue.)
 * @param procID ID of the process to notify.
 */
void notify(ProcID procID);

struct Process *getProcessByID(ProcID id);

ProcID forkProcess(ProcID parentID);

void killCurrentProcess();

void sendMessageTo(ProcID pid, uint8_t msgBoxId, void *message);

struct IRQManager;

const struct IRQManager *getIRQManager_const();

struct IRQManager *getIRQManager();

ProcID processToPID(const struct Process *process);
