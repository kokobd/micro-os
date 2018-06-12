#include <process/Process.h>
#include <ram/PageTable.h>

void Proc_switchPageTable(struct Process *process) {
    uintptr_t root = process->pageTableRoot;
    PageTable_switchTo(root);
}
