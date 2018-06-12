project(kernel)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f elf32 -o <OBJECT> <SOURCE>")

add_executable(kernel
        src/multiboot2.asm
        src/kernel_start.asm
        src/main.c
        include/multiboot2.h

        include/ram/PMM.h
        src/ram/PMM.c
        include/ram/constants.h
        include/ram/PageTable.h
        src/ram/PageTable.c
        include/ram/PMStat.h
        src/ram/PMStat.c

        include/cpu.h
        src/cpu/cpu.c
        src/cpu/8259A.asm
        src/cpu/cpu.asm
        src/cpu/int.asm
        src/cpu/interrupt.c
        include/cpu/interrupt.h
        src/cpu/int.h
        include/cpu/RegState.h
        src/cpu/RegState.c
        src/cpu/IDT.h
        src/cpu/IDT.c

        src/syscall.c
        src/irq.c
        src/exception.c

        include/process/MessageBox.h
        src/process/MessageBox.c
        include/process/Process.h
        src/process/Process.c
        include/process/scheduler.h
        src/process/scheduler.c
        src/process/ProcessQueue.h
        src/process/ProcessQueue.c
        )
target_include_directories(kernel
        PRIVATE include
        )
target_link_libraries(kernel
        core
        )
set_target_properties(kernel PROPERTIES
        LINK_FLAGS "-m32 -nostdlib -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
        LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        )
