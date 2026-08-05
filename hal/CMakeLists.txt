add_library(hal
    i2c_hal.cpp
    spi_hal.cpp
    uart_hal.cpp
    onewire_hal.cpp
)

pico_generate_pio_header(hal ${CMAKE_CURRENT_LIST_DIR}/onewire.pio)

target_include_directories(hal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(hal PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_spi
    hardware_uart
    hardware_pio
    utils
    FreeRTOS-Kernel
)