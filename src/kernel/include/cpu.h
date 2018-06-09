#pragma once

#include <stdint.h>

extern void cpu_initGDT_TSS_8259A();

extern void cpu_enterUserCode(uint32_t target_stack_base)
__attribute__ ((noreturn));

void cpu_initialize();
