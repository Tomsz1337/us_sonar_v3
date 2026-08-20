add_library(drivers STATIC
    bmp085.cpp
    hdc2080.cpp
    neo6m.cpp
)

target_include_directories(drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(drivers PUBLIC
    hal
    FreeRTOS-Kernel
    pico_stdlib
)