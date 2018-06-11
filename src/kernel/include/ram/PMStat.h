#pragma once

#include <stdint.h>
#include <stddef.h>

enum SharePolicy {
    SP_NONE, // do not share
    SP_COW, // copy-on-write strategy
    SP_MEM // shared memory. writable for all sharers.
};

struct FrameStatus {
    enum SharePolicy policy;
    uint32_t processCount;
};

size_t PMStat_size();

void PMStat_init(struct FrameStatus *firstStat);

void PMStat_setPolicy(uintptr_t frame, enum SharePolicy policy);

enum SharePolicy PMStat_getPolicy(uintptr_t frame);

void PMStat_increase(uintptr_t frame);

void PMStat_decrease(uintptr_t frame);

uint32_t PMStat_getProcessCount(uintptr_t frame);

void PMStat_setProcessCount(uintptr_t frame, uint32_t processCount);
