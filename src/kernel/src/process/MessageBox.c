#include <process/MessageBox.h>
#include <string.h>

static const uint8_t *MB_at_const(const struct MessageBox *mb, uint8_t index) {
    return mb->data + ((size_t) index * (size_t) mb->msgSize);
}

static uint8_t *MB_at(struct MessageBox *mb, uint8_t index) {
    return (uint8_t *) MB_at_const(mb, index);
}

static uint8_t MB_next(const struct MessageBox *mb, uint8_t index) {
    ++index;
    if (index == mb->msgMaxNum)
        index = 0;
    return index;
}

bool MB_isFull(const struct MessageBox *mb) {
    if (!MB_isEmpty(mb)) {
        return MB_next(mb, mb->tail) == mb->head;
    } else {
        return false;
    }
}

void MB_push(struct MessageBox *mb, const void *message) {
    if (MB_isEmpty(mb)) {
        mb->tail = mb->head;
    } else {
        mb->tail = MB_next(mb, mb->tail);
    }
    memcpy(MB_at(mb, mb->tail), message, MB_msgSize(mb));
}

void MB_pop(struct MessageBox *mb, void *buffer) {
    memcpy(buffer, MB_at(mb, mb->head), MB_msgSize(mb));
    if (mb->head == mb->tail) {
        mb->tail = MB_msgMaxNum(mb);
    } else {
        mb->head = MB_next(mb, mb->head);
    }
}

void MB_moveData(struct MessageBox *mb, void *newLocation) {
    memcpy(newLocation, mb->data, MB_sizeInBytes(mb));
}

uint8_t length(const struct MessageBox *mb) {
    if (MB_isEmpty(mb)) {
        return 0;
    }

    if (mb->tail > mb->head) {
        return mb->tail - mb->head + (uint8_t) 1;
    }

    if (mb->tail == mb->head) {
        return MB_msgMaxNum(mb);
    }

    // mb->tail < mb->head
    return MB_msgMaxNum(mb) - (mb->head - mb->tail - (uint8_t) 1);
}

void MB_init(struct MessageBox *mb, uint8_t msgSize, uint8_t msgMaxNum) {
    mb->head = 0;
    mb->tail = msgMaxNum;
    mb->msgSize = msgSize;
    mb->msgMaxNum = msgMaxNum;
}

