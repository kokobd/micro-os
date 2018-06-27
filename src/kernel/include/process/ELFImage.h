#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct ELFImage {
    uintptr_t start;
    bool isValid;
    void *highestProgramHeader;
};

void ELFImage_init(struct ELFImage *elfImage, uintptr_t startAddress);

uintptr_t ELFImage_getEntryPoint(const struct ELFImage *elfImage);

size_t ELFImage_requestedSize(struct ELFImage *elfImage);

size_t ELFImage_codeSize(struct ELFImage *elfImage);

inline static bool ELFImage_isValid(const struct ELFImage *elfImage) {
    return elfImage->isValid;
}
