#include <ram/PMStat.h>
#include <ram/PMM.h>
#include <ram/constants.h>

size_t PMStat_size() {
    return pmm_frameCount() * sizeof(struct FrameStatus);
}

static struct FrameStatus *frameStatus;

void PMStat_init(struct FrameStatus *firstStat) {
    frameStatus = firstStat;
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

inline static size_t frameToIndex(uintptr_t frame) {
    return (frame - pmm_firstFrame()) / PAGE_SIZE;
}

inline static struct FrameStatus *frameToEntry(uintptr_t frame) {
    return frameStatus + frameToIndex(frame);
}

void PMStat_setPolicy(uintptr_t frame, enum SharePolicy policy) {
    frameToEntry(frame)->policy = policy;
}

enum SharePolicy PMStat_getPolicy(uintptr_t frame) {
    return frameToEntry(frame)->policy;
}

void PMStat_increase(uintptr_t frame) {
    frameToEntry(frame)->processCount++;
}

void PMStat_decrease(uintptr_t frame) {
    frameToEntry(frame)->processCount--;
}

uint32_t PMStat_getProcessCount(uintptr_t frame) {
    return frameToEntry(frame)->processCount;
}

void PMStat_setProcessCount(uintptr_t frame, uint32_t processCount) {
    frameToEntry(frame)->processCount = processCount;
}
