project(demo)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f elf32 -o <OBJECT> <SOURCE>")

add_executable(demo
        src/main.c
        ../../multiboot2.asm
        )

target_link_libraries(demo
        base
        )

set_target_properties(demo PROPERTIES
        LINK_FLAGS "-m32 -nostdlib -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
        LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        )
