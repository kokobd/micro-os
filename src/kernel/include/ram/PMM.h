#pragma once

#include <multiboot2.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize the physical memory manager
 * @param tag_mmap pointer to a multiboot memory map
 */
bool pmm_init(const struct multiboot_tag_mmap *tag_mmap);

/**
 * Allocate a new page frame.
 * @return pointer to the newly allocated frame
 */
uintptr_t pmm_malloc();

/**
 * Free a page frame
 */
void pmm_free(uintptr_t frame);
