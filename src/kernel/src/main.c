#include <multiboot2.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <ram/PMM.h>
#include <ram/PageTable.h>
#include <ram/constants.h>
#include <cpu.h>
#include <process/scheduler.h>
#include <ram/PMStat.h>

static const struct multiboot_tag_mmap *get_tag_mmap(uint32_t multiboot_info) {
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *) (multiboot_info + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                         + ((tag->size + 7u) & ~7u))) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            return (const struct multiboot_tag_mmap *) tag;
        }
    }
    return NULL;
}

void main(uint32_t magic_number, uint32_t address) {
    cpu_initialize();
    pmm_init(get_tag_mmap(address));
    if (pmm_firstFrame() >= PT_VADDR || pmm_firstFrame() >= address)
        return;

    PageTable_enablePaging();

    PageTable_allocateGlobally(KERNEL_GLOBAL_DATA, pmm_frameCount() * sizeof(struct FrameStatus));
    uintptr_t kernelPT = PageTable_new();
    PageTable_switchTo(kernelPT);

    PMStat_init((struct FrameStatus *) KERNEL_GLOBAL_DATA);

    initScheduler();
    addInitialProcess(0x500); // demo
    addInitialProcess(0x4100); // idle

    cpu_enterUserCode(KERNEL_STACK_BASE);
}

