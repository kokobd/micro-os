#include <cpu.h>
#include "IDT.h"
#include "int.h"

void cpu_initialize() {
    cpu_initGDT_TSS_8259A();

    for (uint8_t i = 0; i < HANDLERS_COUNT; ++i) {
        uint8_t dpl = 0;
        // system call should have a dpl of 3
        if (i == HANDLERS_COUNT - 1)
            dpl = 3;
        void (*handler)() = (void (*)())
                ((uintptr_t) cpu_intHandlerPrim + i * cpu_intHandlerPrimEntrySize);
        IDT_setHandler(i, dpl, 0x08, handler);
    }
    IDT_install();
}

