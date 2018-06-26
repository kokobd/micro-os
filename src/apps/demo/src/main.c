#include <base/sys/unistd.h>

int main() {
    char *heap = sbrk(0);
    sbrk(PAGE_SIZE);

    mapPhysicalMemory((uintptr_t) heap, 0xB8000);
    *heap = 'A';
    return 0;
}
