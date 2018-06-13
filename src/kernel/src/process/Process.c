#include <process/Process.h>
#include <ram/PageTable.h>
#include <ram/constants.h>
#include <ram/PMM.h>
#include <core/utility.h>
#include <string.h>

void Process_applyPageTable(struct Process *process) {
    uintptr_t root = process->pageTableRoot;
    PageTable_switchTo(root);
}

enum ProcMemRegion {
    CODE_HEAP, STACK, OTHER
};

static enum ProcMemRegion checkAddress(uintptr_t programBreak, uintptr_t address) {
    if (PROC_BEGIN <= address && address < programBreak) {
        return CODE_HEAP;
    } else if (PROC_STACK_BASE - PROC_STACK_SIZE <= address
               && address < PROC_STACK_BASE) {
        return STACK;
    } else {
        return OTHER;
    }
}

static bool increaseProgramBreak(uintptr_t *pb_p, size_t size) {
    const uintptr_t pb = *pb_p;
    const uintptr_t pb_page = core_alignDown(pb, PAGE_SHIFT);
    const uintptr_t pb_new = pb + size;
    if (pb < PROC_STACK_BASE - PROC_STACK_SIZE) {
        const uintptr_t pb_page_new = core_alignDown(pb, PAGE_SHIFT);
        for (uintptr_t page = pb_page + PAGE_SIZE; page <= pb_page_new; page += PAGE_SIZE) {
            uintptr_t frame = pmm_malloc();
            if (frame == 0) {
                // Allocation failed, we need to free all previously allocated frames.
                for (uintptr_t page_to_free = pb_page + PAGE_SIZE;
                     page_to_free < page;
                     page_to_free += PAGE_SIZE) {
                    uintptr_t frame_to_free = PageTable_getMapping(page_to_free);
                    pmm_free(frame_to_free);
                }
                return false;
            }
            PageTable_setMapping(page, frame);
        }
        *pb_p = pb_new;
        return true;
    } else {
        return false;
    }
}

static bool decreaseProgramBreak(uintptr_t *pb_p, size_t size) {
    const uintptr_t pb = *pb_p;
    const uintptr_t pb_page = core_alignDown(pb, PAGE_SHIFT);
    const uintptr_t pb_new = pb - size;
    const uintptr_t pb_page_new = core_alignDown(pb_new, PAGE_SHIFT);

    bool valid = false;
    switch (checkAddress(pb, pb_new)) {
        case CODE_HEAP:
            valid = true;
            break;
        default:
            break;
    }

    if (valid) {
        for (uintptr_t page = pb_page; page > pb_page_new; page -= PAGE_SIZE) {
            uintptr_t frame = PageTable_getMapping(page);
            pmm_free(frame);
            PageTable_clearMapping(page);
        }
        *pb_p = pb_new;
    }

    return valid;
}

void Process_createWithImage(struct Process *process, uintptr_t imagePAddr,
                             size_t imageSize, size_t requestedSize,
                             uintptr_t entryPoint) {
    RegState_init(&process->regState);

    process->pageTableRoot = PageTable_new();
    Process_applyPageTable(process);

    increaseProgramBreak(&process->programBreak, requestedSize);
    memcpy((void *) PROC_BEGIN, (const void *) imagePAddr, imageSize);
    // we do not need to create the stack, as the stack will
    // be created on demand.
    process->regState.eip = entryPoint;
    process->regState.esp = PROC_STACK_BASE;
    process->regState.ebp = PROC_STACK_BASE;
}

static void releaseStackMem() {
    for (uintptr_t page = PROC_STACK_BASE - PROC_STACK_SIZE;
         page < PROC_STACK_BASE; page += PAGE_SIZE) {
        uintptr_t frame = PageTable_getMapping(page);
        if (frame != 0) {
            PageTable_clearMapping(page);
            pmm_free(frame);
        }
    }
}

void Process_exit(struct Process *process) {
    PageTable_switchTo(process->pageTableRoot);
    // release code and heap
    decreaseProgramBreak(&process->programBreak, process->programBreak - PROC_BEGIN);
    // release stack
    releaseStackMem();

    PageTable_free();
}
