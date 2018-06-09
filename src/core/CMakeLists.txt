project(core)

add_library(core STATIC
        src/ctype.c
        src/string.c

        include_std/ctype.h
        include_std/string.h

        include/core/utility.h
        include/core/BitSet.h
        src/core/BitSet.c
        )

target_include_directories(core
        PUBLIC include_std include
        PRIVATE src
        )

target_compile_options(core
        PUBLIC $<$<COMPILE_LANGUAGE:C>:-masm=intel -ffreestanding -fno-stack-protector>
        )
