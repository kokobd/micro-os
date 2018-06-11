#include <ram/PMStat.h>
#include <ram/PMM.h>
#include <ram/constants.h>

size_t PMStat_size() {
    return pmm_frameCount() * sizeof(struct FrameStatus);
}

static struct FrameStatus *frameStatus;

void PMStat_init(uintptr_t location) {
    frameStatus = (struct FrameStatus *) location;
    size_t frameCount = pmm_frameCount();
    uintptr_t firstFrame = pmm_firstFrame();
    for (size_t i = 0; i != frameCount; ++i) {
        frameStatus[i].policy = SP_NONE;

        if (pmm_isAvailable(firstFrame + i * PAGE_SIZE)) {
            frameStatus[i].processCount = 0;
        } else {
            frameStatus[i].processCount = 1;
            /* so that the frame is always considered as occupied,
             * but no process will return it. This effectively marked
             * it as none available.
             */
        }
    }
}


