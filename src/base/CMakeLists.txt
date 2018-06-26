project(base)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f elf32 -o <OBJECT> <SOURCE>")

add_library(base STATIC
        src/crt.c
        src/crt.asm

        include/base/sys/ipc.h
        src/sys/ipc.c
        src/sys/syscall.h
        src/sys/syscall.c
        include/base/sys/unistd.h
        src/sys/unistd.c
        )

target_include_directories(base
        PUBLIC include
        )

target_link_libraries(base
        PUBLIC core)
