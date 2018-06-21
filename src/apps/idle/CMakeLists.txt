project(idle)

add_executable(idle
        src/main.c
        )

target_link_libraries(idle
        base
        )

set_target_properties(idle PROPERTIES
        LINK_FLAGS "-m32 -nostdlib -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld"
        LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld
        )
