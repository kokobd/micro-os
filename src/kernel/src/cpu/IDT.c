#include "IDT.h"

#include <stddef.h>
#include <stdbool.h>

static IDTDesc idt[HANDLERS_COUNT];

static IDTR idtr;

bool IDT_setHandler(uint8_t index, uint8_t dpl, uint16_t selectorInGDT,
                    void (*handler)()) {
    if (index >= HANDLERS_COUNT || handler == NULL)
        return false;

    uint32_t handlerAddress = (uint32_t) handler;
    idt[index].baseLo = (uint16_t) (handlerAddress & 0x0000FFFFu);
    idt[index].baseHi = (uint16_t) ((handlerAddress >> 16u) & 0xFFFFu);
    idt[index].reserved = 0;
    idt[index].sel = selectorInGDT;
    idt[index].flags = (uint8_t) (dpl << 5u) | (uint8_t) 0b10001110u;
    return true;
}

void IDT_install() {
    idtr.base = (uint32_t) idt;
    idtr.limit = sizeof(idt);
    asm volatile (
    "cli\n"
    "lidt [eax]\n"
    : : "a"(&idtr)
    );
}
