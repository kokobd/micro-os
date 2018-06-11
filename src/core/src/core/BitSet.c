#include <core/BitSet.h>
#include <string.h>
#include <limits.h>

void core_BitSet_init(core_BitSet *bitSet, bool init) {
    size_t count;
    if (bitSet->sizeInBits % 8 == 0) {
        count = bitSet->sizeInBits / 8;
    } else {
        count = bitSet->sizeInBits / 8 + 1;
    }
    memset(bitSet->data, init ? UINT8_MAX : 0, count);
}

bool core_BitSet_set(core_BitSet *bitSet, size_t index, bool value) {
    if (index >= bitSet->sizeInBits)
        return false;
    if (value) {
        bitSet->data[index / 32u] |= (1u << (index % 32u));
    } else {
        bitSet->data[index / 32u] &= ~(1u << (index % 32u));
    }
    return true;
}

size_t core_BitSet_length(core_BitSet *bitSet) {
    return bitSet->sizeInBits;
}

size_t core_BitSet_findFirst(core_BitSet *bitSet, bool value) {
    const size_t sizeInUInt32 = bitSet->sizeInBits / 32;
    if (value) {
        for (size_t i = 0; i != sizeInUInt32; ++i) {
            if (bitSet->data[i]) {
                size_t len = i * 32u;
                uint32_t mask = 1;
                while (mask != 0) {
                    if (bitSet->data[i] & mask) {
                        return len;
                    }
                    ++len;
                    mask = mask << 1u;
                }
                // Impossible code path
                return len;
            }
        }
    } else {
        for (size_t i = 0; i != sizeInUInt32; ++i) {
            if (~bitSet->data[i]) {
                size_t len = i * 32u;
                uint32_t mask = 1;
                while (mask != 0) {
                    if (~bitSet->data[i] & mask) {
                        return len;
                    }
                    ++len;
                    mask = mask << 1u;
                }
                // Impossible code path
                return len;
            }
        }
    }

    return bitSet->sizeInBits;
}

bool core_BitSet_get(core_BitSet *bitSet, size_t index) {
    return (bool) (bitSet->data[index / 32u] & (1u << (index % 32u)));
}
