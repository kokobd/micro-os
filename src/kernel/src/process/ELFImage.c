#include <process/ELFImage.h>

static bool checkELFSignature(uintptr_t startAddress) {
    uint8_t *start = (uint8_t *) startAddress;
    // check the first four bytes against the magic number
    return start[0] == 0x7F
           && start[1] == 0x45
           && start[2] == 0x4C
           && start[3] == 0x46;
}

struct ELFProgramHeader {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    // size of the segment in file
    uint32_t fileSize;
    // size of the segment in memory
    uint32_t memorySize;
    uint32_t flags;
    uint32_t align;
};

#define PT_NULL 0
#define PT_LOAD 1

static const struct ELFProgramHeader *programHeaders_const(const struct ELFImage *elfImage) {
    ptrdiff_t phoff = *(uint32_t *) (elfImage->start + 0x1C);
    return (const struct ELFProgramHeader *) (elfImage->start + phoff);
}

static struct ELFProgramHeader *programHeaders(struct ELFImage *elfImage) {
    return (struct ELFProgramHeader *) programHeaders_const(elfImage);
}

static uint16_t programHeadersCount(const struct ELFImage *elfImage) {
    return *(uint16_t *) (elfImage->start + 0x2C);
}

static const struct ELFProgramHeader *highestProgramHeader(
        struct ELFImage *elfImage) {
    if (elfImage->highestProgramHeader == NULL) {
        uint16_t phCount = programHeadersCount(elfImage);
        struct ELFProgramHeader *headers = programHeaders(elfImage);
        uintptr_t ph_vaddr = 0;
        for (uint16_t i = 0; i < phCount; ++i) {
            if (headers[i].type == PT_LOAD) {
                if (headers[i].vaddr > ph_vaddr) {
                    ph_vaddr = headers[i].vaddr;
                    elfImage->highestProgramHeader = headers + i;
                }
            }
        }
    }

    return elfImage->highestProgramHeader;
}

void ELFImage_init(struct ELFImage *elfImage, uintptr_t startAddress) {
    elfImage->start = startAddress;
    elfImage->isValid = checkELFSignature(startAddress);
    elfImage->highestProgramHeader = NULL;
}

uintptr_t ELFImage_getEntryPoint(const struct ELFImage *elfImage) {
    if (!ELFImage_isValid(elfImage))
        return 0;

    return *(uint32_t *) (elfImage->start + 0x18);
}

size_t ELFImage_requestedSize(struct ELFImage *elfImage) {
    const struct ELFProgramHeader *ph = highestProgramHeader(elfImage);
    return ph->vaddr + ph->memorySize - elfImage->start;
}

size_t ELFImage_codeSize(struct ELFImage *elfImage) {
    const struct ELFProgramHeader *ph = highestProgramHeader(elfImage);
    return ph->vaddr + ph->fileSize - elfImage->start;
}
