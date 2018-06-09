#include <ram/PMM.h>
#include <ram/constants.h>
#include <core/utility.h>
#include <core/BitSet.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static core_BitSet availableFrames;

static uint64_t maxPAddr(const struct multiboot_mmap_entry *entries, uint32_t entry_count) {
    uint64_t maxValue = 0;
    for (uint32_t i = 0; i != entry_count; ++i) {
        if (entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            const uint64_t x = entries[i].addr + entries[i].len;
            if (x > maxValue)
                maxValue = x;
        }
    }
    return maxValue - 1;
}

static bool isMemoryValid(const struct multiboot_mmap_entry *entries, uint32_t entry_count) {
    for (uint32_t i = 0; i != entry_count; ++i) {
        if (entries[i].type == MULTIBOOT_MEMORY_AVAILABLE
            && entries[i].addr == PMM_BEGIN) {
            if (entries[i].len > (availableFrames.sizeInBits >> 3u)) {
                return true;
            }
        }
    }
    return false;
}

static size_t addressToIndex(uintptr_t paddr) {
    return (paddr - PMM_BEGIN) >> PAGE_SHIFT;
}

static uintptr_t indexToAddress(uintptr_t index) {
    return (index << PAGE_SHIFT) + PMM_BEGIN;
}

static bool page_status_init(const struct multiboot_mmap_entry *entries, uint32_t entry_count) {
    bool valid = isMemoryValid(entries, entry_count);

    if (valid) {
        availableFrames.data = (uint32_t *) PMM_BEGIN;
        core_BitSet_init(&availableFrames, false);

        for (uint32_t i = 0; i != entry_count; ++i) {
            if (entries[i].type == MULTIBOOT_MEMORY_AVAILABLE
                && entries[i].addr >= PMM_BEGIN) {
                size_t from = addressToIndex(core_alignUp((uintptr_t) (entries[i].addr), PAGE_SHIFT));
                size_t to = addressToIndex(core_alignDown((uintptr_t) (entries[i].addr + entries[i].len), PAGE_SHIFT));
                for (size_t j = from; j < to; ++j) {
                    core_BitSet_set(&availableFrames, j, true);
                }
            }
        }
    }

    // Mark frames occupied by meta-info as not available
    size_t begin = 0;
    size_t end = core_alignUp(core_BitSet_length(&availableFrames) / 8u, PAGE_SHIFT)
            >> PAGE_SHIFT;
    while (begin < end) {
        core_BitSet_set(&availableFrames, begin, false);
        ++begin;
    }

    return valid;
}

bool pmm_init(const struct multiboot_tag_mmap *tag_mmap) {
    const uint32_t entry_count = (tag_mmap->size - 4 * sizeof(uint32_t)) / tag_mmap->entry_size;

    uint64_t mem_end = maxPAddr(tag_mmap->entries, entry_count) + 1;
    mem_end = core_alignDown(mem_end, PAGE_SHIFT);

    availableFrames.sizeInBits = (size_t) ((mem_end - PMM_BEGIN) >> PAGE_SHIFT);

    return page_status_init(tag_mmap->entries, entry_count);
}

uintptr_t pmm_malloc() {
    size_t index = core_BitSet_findFirst(&availableFrames, true);
    if (index == availableFrames.sizeInBits)
        return 0;

    uintptr_t paddr = indexToAddress(index);
    core_BitSet_set(&availableFrames, index, false);
    return paddr;
}

void pmm_free(uintptr_t frame) {
    if (frame != 0) {
        size_t index = addressToIndex(frame);
        core_BitSet_set(&availableFrames, index, true);
    }
}

