#pragma once

#include <stdint.h>

int maxMsgBoxId();

enum MsgBoxError {
    MBE_NO_ERROR        = 0,
    MBE_INVALID_ID      = 1,
    MBE_NOT_INITIALIZED = 2,
    MBE_INVALID_ARGS    = 3
};

struct MsgBoxInfo {
    void    *memory; // buffer to store messages.
    uint8_t msgSize;
    uint8_t msgMaxCount;
};

enum MsgBoxError initMsgBox(int id, const struct MsgBoxInfo *);