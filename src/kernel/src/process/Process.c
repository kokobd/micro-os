#include <process/Process.h>
#include <ram/PageTable.h>
#include <ram/constants.h>
#include <ram/PMM.h>
#include <core/utility.h>
#include <string.h>
#include <ram/PMStat.h>

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
            PMStat_setPolicy(frame, SP_NONE);
            PMStat_setProcessCount(frame, 1);
            PageTable_setMapping(page, frame);
        }
        *pb_p = pb_new;
        return true;
    } else {
        return false;
    }
}

static void releaseFrame(uintptr_t frame) {
    PMStat_decrease(frame);
    if (PMStat_getProcessCount(frame) == 0) {
        pmm_free(frame);
    }
}

inline static bool hasIntersection(uintptr_t a_beg, uintptr_t a_end, uintptr_t b_beg, uintptr_t b_end) {
    return !(a_end <= b_beg || b_end <= a_beg);
}

static bool hasMessageBox(const struct MessageBox *msgBoxes, size_t count, uintptr_t beg, uintptr_t end) {
    for (size_t i = 0; i < count; ++i) {
        const struct MessageBox *msgBox = msgBoxes + i;
        if (MB_isValid(msgBox)) {
            if (hasIntersection(beg, end,
                                (uintptr_t) msgBoxes->data,
                                (uintptr_t) (msgBox->data + MB_sizeInBytes(msgBox)))) {
                return true;
            }
        }
    }
    return false;
}

static bool decreaseProgramBreak(struct Process *process, size_t size) {
    const uintptr_t pb = process->programBreak;
    const uintptr_t pb_page = core_alignDown(pb, PAGE_SHIFT);
    const uintptr_t pb_new = pb - size;
    const uintptr_t pb_page_new = core_alignDown(pb_new, PAGE_SHIFT);

    bool valid = true;
    if (checkAddress(pb, pb_new) != CODE_HEAP)
        valid = false;
    if (hasMessageBox(process->msgBoxes, MSGBOX_LIMIT, pb_new, pb))
        valid = false;

    if (valid) {
        for (uintptr_t page = pb_page; page > pb_page_new; page -= PAGE_SIZE) {
            uintptr_t frame = PageTable_getMapping(page);
            releaseFrame(frame);
            PageTable_clearMapping(page);
        }
        process->programBreak = pb_new;
    }

    return valid;
}

static void initMsgBoxesInvalid(struct MessageBox *msgBoxes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        MB_initInvalid(msgBoxes + i);
    }
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

    initMsgBoxesInvalid(process->msgBoxes, MSGBOX_LIMIT);
}

static void releaseStackMem() {
    for (uintptr_t page = PROC_STACK_BASE - PROC_STACK_SIZE;
         page < PROC_STACK_BASE; page += PAGE_SIZE) {
        uintptr_t frame = PageTable_getMapping(page);
        if (frame != 0) {
            PageTable_clearMapping(page);
            releaseFrame(frame);
        }
    }
}

void Process_exit(struct Process *process) {
    process->status.type = PS_INVALID;

    PageTable_switchTo(process->pageTableRoot);
    // release code and heap
    decreaseProgramBreak(process, process->programBreak - PROC_BEGIN);
    // release stack
    releaseStackMem();

    PageTable_free();
}

void Process_fork(struct Process *parent, struct Process *child) {
    child->regState = parent->regState;
    child->programBreak = parent->programBreak;
    child->status = parent->status;
    initMsgBoxesInvalid(child->msgBoxes, MSGBOX_LIMIT);

    uintptr_t prevPT = PageTable_currentRoot();
    // Copy the page table. Employ copy on write strategy.
    PageTable_switchToID();
    child->pageTableRoot = PageTable_copy(parent->pageTableRoot);
    PageTable_switchTo(child->pageTableRoot);
    uintptr_t heapEnd = core_alignUp(child->programBreak, PAGE_SHIFT);

    for (uintptr_t page = PROC_BEGIN; page < PROC_STACK_BASE;
         page += PAGE_SIZE) {
        if (page == heapEnd) {
            page = PROC_STACK_BASE - PROC_STACK_SIZE;
        }
        uintptr_t frame = PageTable_getMapping(page);
        if (frame == 0)
            continue;
        enum SharePolicy frameSP = PMStat_getPolicy(frame);

        if (frameSP == SP_NONE) {
            frameSP = SP_COW;
            PMStat_setPolicy(frame, frameSP);
        }
        PMStat_increase(frame);
        PageTable_setMapping(page, frame);

        if (frameSP == SP_COW) {
            // Set the page as readonly.
            PageTable_removeAttr(page, PA_WRITABLE);
            PageTable_with(parent->pageTableRoot, {
                PageTable_removeAttr(page, PA_WRITABLE);
            });
        }
    }
    PageTable_switchTo(prevPT);
}

struct MessageBox *Process_msgBox(struct Process *process, int msgBoxID) {
    if (msgBoxID < 0 || msgBoxID >= MSGBOX_LIMIT) {
        return NULL;
    }
    return process->msgBoxes + msgBoxID;
}

bool Process_ownMemory(struct Process *process, uintptr_t begin, size_t size) {
    bool ret = true;

    PageTable_with(process->pageTableRoot, {
        uintptr_t begin_ = core_alignDown(begin, PAGE_SHIFT);
        uintptr_t end_ = core_alignUp(begin + size, PAGE_SHIFT);

        for (uintptr_t page = begin_; page < end_; page += PAGE_SIZE) {
            if (PageTable_getMapping(page) == 0 ||
                PageTable_getAttr(page) & PA_USER_MODE == 0) {
                ret = false;
                break;
            }
        }
    });
    return ret;
}

void Process_replaceMe(struct Process *process, uintptr_t image, size_t size, uintptr_t entryPoint) {
    size_t size_ = core_alignUp(size, PAGE_SHIFT);
    PageTable_with(process->pageTableRoot, {
        // move the image
        const ptrdiff_t shift = image - PROC_BEGIN;
        for (uintptr_t page = image; page != image + size_; page += PAGE_SIZE) {
            PageTable_swapPages(page, page - shift);
        }
        decreaseProgramBreak(process, process->programBreak - (PROC_BEGIN + size));
        releaseStackMem();
        MB_initInvalid(process->msgBoxes);
    });

    RegState_init(&process->regState);
    process->regState.eip = entryPoint;
}

uintptr_t Process_sbrk(struct Process *process, ptrdiff_t diff) {
    if (diff != 0) {
        PageTable_with(process->pageTableRoot, {
            if (diff > 0) {
                increaseProgramBreak(&process->programBreak, (size_t) diff);
            } else if (diff < 0) {
                decreaseProgramBreak(process, (size_t) -diff);
            }
        });
    }
    return process->programBreak;
}
