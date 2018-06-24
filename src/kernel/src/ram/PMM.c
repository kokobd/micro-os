#include <ram/PMM.h>
#include <ram/constants.h>
#include <core/utility.h>
#include <core/BitSet.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static core_BitSet availableFrames;

static uintptr_t firstFrame;

static uintptr_t idPageTable;

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

static size_t addressToIndex(uintptr_t paddr) {
    return (paddr - PMM_BEGIN) >> PAGE_SHIFT;
}

static uintptr_t indexToAddress(uintptr_t index) {
    return (index << PAGE_SHIFT) + PMM_BEGIN;
}

static uint64_t firstHighMemoryEnd(const struct multiboot_mmap_entry *entries, uint32_t entry_count) {
    for (uint32_t i = 0; i < entry_count; ++i) {
        if (entries[i].addr == PMM_BEGIN) {
            return entries[i].addr + entries[i].len;
        }
    }
    return PMM_BEGIN;
}

static bool page_status_init(const struct multiboot_mmap_entry *entries, uint32_t entry_count) {
    // Calculate static memory usage of the bitset
    const size_t bitSetSize =
            core_alignUp(core_BitSet_length(&availableFrames) / 8, PAGE_SHIFT);
    idPageTable = PMM_BEGIN + bitSetSize;
    // Calculate static memory usage of the identity mapping
    // page table
    const size_t idPTSize =
            (core_divUp(core_BitSet_length(&availableFrames) - PMM_BEGIN / PAGE_SIZE, 1024)
             + 1) * PAGE_SIZE;
    firstFrame = idPageTable + idPTSize;

    // Validate that the first high memory block is
    // sufficient for the bitset and id page table
    if (firstFrame >= firstHighMemoryEnd(entries, entry_count)) {
        return false;
    }

    // Init the bitset. It is stored at the
    // beginning of 1MiB
    availableFrames.data = (uint32_t *) PMM_BEGIN;
    core_BitSet_init(&availableFrames, false);

    // Mark detected available frames.
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

    // Mark frames occupied by the bitset and id page table
    // as not available
    size_t begin = 0;
    size_t end = (bitSetSize + idPTSize) / PAGE_SIZE;
    while (begin < end) {
        core_BitSet_set(&availableFrames, begin, false);
        ++begin;
    }

    return true;
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

uintptr_t pmm_firstFrame() {
    return firstFrame;
}

uintptr_t pmm_idPageTable() {
    return idPageTable;
}

size_t pmm_frameCount() {
    return core_BitSet_length(&availableFrames);
}

bool pmm_isAvailable(uintptr_t frame) {
    uint32_t i = addressToIndex(frame);
    return core_BitSet_get(&availableFrames, i);
}

bool pmm_requireFrame(uintptr_t frame) {
    if (frame <= pmm_firstFrame())
        return false;
    uint32_t i = addressToIndex(frame);
    if (!pmm_isAvailable(frame))
        return false;
    core_BitSet_set(&availableFrames, i, false);
    return true;
}
