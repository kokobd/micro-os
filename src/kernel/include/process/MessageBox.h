#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct MessageBox {
    uint8_t msgSize;
    uint8_t msgMaxNum;
    uint8_t head; // points to the first message
    uint8_t tail; // points to the last message
    uint8_t *data; // where the messages are stored
};

void MB_init(struct MessageBox *mb, uint8_t msgSize, uint8_t msgMaxNum);

inline static uint8_t MB_msgSize(struct MessageBox *mb) {
    return mb->msgSize;
}

inline static uint8_t MB_msgMaxNum(struct MessageBox *mb) {
    return mb->msgMaxNum;
}

inline static bool MB_isEmpty(const struct MessageBox *mb) {
    return mb->tail == mb->msgMaxNum;
}

bool MB_isFull(const struct MessageBox *mb);

void MB_push(struct MessageBox *mb, const void *message);

void MB_pop(struct MessageBox *mb, void *buffer);

void MB_moveData(struct MessageBox *mb, void *newLocation);

inline static size_t MB_sizeInBytes(struct MessageBox *mb) {
    return (size_t) MB_msgSize(mb) * MB_msgMaxNum(mb);
}

uint8_t length(struct MessageBox *mb);

