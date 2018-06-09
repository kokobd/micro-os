#include <multiboot2.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <ram/PMM.h>
#include <cpu.h>

static const struct multiboot_tag_mmap *get_tag_mmap(uint32_t multiboot_info) {
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *) (multiboot_info + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                         + ((tag->size + 7u) & ~7u))) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            pmm_init((const struct multiboot_tag_mmap *) tag);
        }
    }
}

void main(uint32_t magic_number, uint32_t address) {
    cpu_initialize();
    pmm_init(get_tag_mmap(address));

    asm volatile("int 0x30");
    asm volatile("nop");
}

