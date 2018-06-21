#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main(int argc, char *argv[]) {
    while (true) {
        asm volatile ("nop");
    }
}

#pragma clang diagnostic pop
