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

void PMStat_init(uintptr_t location);
