#pragma once

#include <stdint.h>
#include <stdbool.h>

#pragma pack (push, 1)
typedef struct {
    // lower 16 bits of IR address
    uint16_t baseLo;

    // selector in GDT
    uint16_t sel;

    // reserved, should be 0
    uint8_t reserved;

    // bit flags
    // should be 1__01110 (for interrupt gate), where the underlines denotes the DPL
    uint8_t flags;

    // higher 16 bits of IR address
    uint16_t baseHi;
} IDTDesc;

typedef struct {
    // size of the IDT
    uint16_t limit;

    // base address of the IDT
    uint32_t base;
} IDTR;
#pragma pack (pop)

bool IDT_setHandler(uint8_t index, uint8_t dpl, uint16_t selectorInGDT,
                    void (*handler)());

void IDT_install();

#define HANDLERS_COUNT 0x31