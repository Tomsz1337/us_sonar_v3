add_library(hal
    i2c_hal.cpp
)

target_include_directories(hal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(hal PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_gpio
    utils
    FreeRTOS-Kernel
)