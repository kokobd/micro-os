#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * Enable paging and apply the identity mapping page table.
 */
void PageTable_enablePaging();

/**
 * Subsequently created page tables will have a shared, writable,
 * kernel-mode memory region. Note that it won't present in ID map.
 * @param vaddr must be aligned to 4MiB boundary
 * @param size size of the requested memory region.
 */
void PageTable_allocateGlobally(uintptr_t vaddr, size_t size);

/**
 * Precondition: (Paging not enabled) OR (Identity mapping is in effect)
 * @return Root frame of the newly created page table
 */
uintptr_t PageTable_new();

/**
 * Precondition: Identity mapping is in effect.
 * @param srcPD physical address of the page directory table
 * @return page directory table of the newly created page table.
 */
uintptr_t PageTable_copy(uintptr_t srcPD);

/**
 * Destruct current page table and switch to ID page table.
 * This function will NOT release page frames passed in with 'setMapping'.
 */
void PageTable_free();

void PageTable_switchTo(uintptr_t rootFrame);

void PageTable_switchToID();

void PageTable_refresh();

/**
 * Set a page's frame of the currently in-effect page table.
 * You can not change the meta page
 * @param vaddr page address
 * @param paddr frame address
 * @return whether the operation succeeded
 */
bool PageTable_setMapping(
        uintptr_t vaddr,
        uintptr_t paddr);

uintptr_t PageTable_getMapping(uintptr_t vaddr);

enum PageAttr {
    PA_NONE      = 0,
    PA_WRITABLE  = 0b01,
    PA_USER_MODE = 0b10
};

/**
 * Set a page's attribute. See PageTable_setMapping.
 */
void PageTable_setAttr(
        uintptr_t vaddr,
        enum PageAttr attr);

enum PageAttr PageTable_getAttr(uintptr_t vaddr);

void PageTable_removeAttr(uintptr_t page, enum PageAttr attr);

/**
 * Remove a page
 * @param vaddr page address
 */
void PageTable_clearMapping(
        uintptr_t vaddr);

uintptr_t PageTable_currentRoot();

#define PageTable_with(pageTable, CODE)\
do {\
    uintptr_t curPTRoot = PageTable_currentRoot();\
    PageTable_switchTo((pageTable));\
    CODE\
    PageTable_switchTo(curPTRoot);\
} while (0)
