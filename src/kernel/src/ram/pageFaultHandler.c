#include <process/scheduler.h>
#include <cpu/RegState.h>
#include <ram/constants.h>
#include <ram/PMM.h>
#include <ram/PageTable.h>
#include <core/utility.h>
#include <ram/PMStat.h>
#include <string.h>

void onPageFault() {
    // Possible cases:
    //   - invalid memory location (not in code-heap, not in the stack, and not a share page): kill
    //   - valid location, but:
    //       - read-only --> must be copy-on-write
    //       - in the stack --> need to allocate page frame for that page.
    //       - otherwise, kill the process

    const uint32_t pfErrCode = currentProcess()->regState.errorCode;

    bool present = (bool) (pfErrCode & 0b1u);
    bool write = (bool) (pfErrCode & 0b10u);
    bool user = (bool) (pfErrCode & 0b100u);
    bool insFetch = (bool) (pfErrCode & 0b1000u);

    uintptr_t vaddr;
    asm volatile (
    "mov %0, cr2"
    : "=rm" (vaddr)
    );
    uintptr_t page = core_alignDown(vaddr, PAGE_SHIFT);

    enum PageAttr pageAttr = PageTable_getAttr(page);

    if (!present) {
        // if in the stack, allocate memory
        // otherwise, kill the process
        if (vaddr >= PROC_STACK_BASE - PROC_STACK_SIZE && vaddr < PROC_STACK_BASE) {
            uintptr_t frame = pmm_malloc();
            PMStat_increase(frame);
            PageTable_setMapping(page, frame);
            PageTable_setAttr(page, PA_WRITABLE | PA_USER_MODE);
        } else {
            killCurrentProcess();
        }
    } else {
        if (!PageTable_hasAttr(page, PA_USER_MODE)) {
            killCurrentProcess();
        } else if (write) {
            // This is a user page, but read-only.
            // It must be the case of COW strategy.
            uintptr_t frame = PageTable_getMapping(page);

            if (PMStat_getPolicy(frame) == SP_COW) {
                if (PMStat_getProcessCount(frame) == 1) {
                    PMStat_setPolicy(frame, SP_NONE);
                    PageTable_addAttr(page, PA_WRITABLE);
                } else {
                    PMStat_decrease(frame);
                    uintptr_t newFrame = pmm_malloc();
                    PMStat_increase(newFrame);
                    PageTable_with(PageTable_id(), {
                        memcpy((void *) newFrame, (void *) frame, PAGE_SIZE);
                    });
                    PageTable_setMapping(page, newFrame);
                }
            } else {
                killCurrentProcess();
            }
        }
    }
}
