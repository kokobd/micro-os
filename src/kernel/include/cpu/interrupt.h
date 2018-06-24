#pragma once

#include "RegState.h"

extern void cpu_syscallHandler();

#define IRQ_COUNT 16u

extern void cpu_extIntHandler(uint32_t irq_num);

enum cpu_Exception {
    CPU_EXC_DE = 0x0, // Divide-by-zero Error
    CPU_EXC_DB = 0x1, // Debug
    CPU_EXC_NI = 0x2, // Non-maskable Interrupt
    CPU_EXC_BP = 0x3, // Breakpoint
    CPU_EXC_OF = 0x4, // Overflow
    CPU_EXC_BR = 0x5, // Bound Range Exceeded
    CPU_EXC_UD = 0x6, // Invalid Opcode
    CPU_EXC_NM = 0x7, // Device Not Available
    CPU_EXC_DF = 0x8, // Double Fault
    CPU_EXC_TS = 0xA, // Invalid TSS
    CPU_EXC_NP = 0xB, // Segment not Present
    CPU_EXC_SS = 0xC, // Stack-Segment Fault
    CPU_EXC_GP = 0xD, // General Protection Fault
    CPU_EXC_PF = 0xE, // Page Fault
    CPU_EXC_MF = 0x10, // x87 Floating-Point Exception
    CPU_EXC_AC = 0x11, // Alignment Check
    CPU_EXC_MC = 0x12, // Machine Check
    CPU_EXC_XM = 0x13, // SIMD Floating-Point Exception
    CPU_EXC_VE = 0x14, // Virtualization Exception
    CPU_EXC_SX = 0x1E // Security Exception
};

/**
 * CPU internal exception handler
 * @param exception the exception type
 * @param regState pointer to the saved register state on stack
 */
extern void cpu_excHandler(enum cpu_Exception exception);
