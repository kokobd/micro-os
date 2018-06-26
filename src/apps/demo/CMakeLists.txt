project(demo)

add_executable(demo
        src/main.c
        )

target_link_libraries(demo
        base
        )

set_target_properties(demo PROPERTIES
        LINK_FLAGS "-m32 -nostdlib -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld"
        LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld
        )
