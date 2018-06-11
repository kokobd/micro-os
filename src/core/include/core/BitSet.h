#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint32_t *data;
    size_t sizeInBits;
} core_BitSet;

/**
 * Initialize a BitSet. The caller must provide a 4-byte aligned memory
 * region that has at lease or more than 'sizeInBits' bits.
 * @param bitSet the bitset to initialize
 * @param sizeInBits max number of bits
 * @param init initial value
 */
void core_BitSet_init(core_BitSet *bitSet, bool init);

/**
 * Set bits in [from, to) to a specific value
 * @param index bit index
 * @return whether the operation succeeded
 */
bool core_BitSet_set(core_BitSet *bitSet, size_t index, bool value);

bool core_BitSet_get(core_BitSet *bitSet, size_t index);

size_t core_BitSet_length(core_BitSet *bitSet);

size_t core_BitSet_findFirst(core_BitSet *bitSet, bool value);
