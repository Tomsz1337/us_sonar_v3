add_executable(i2c_test
    i2c_scanner.cpp
)

target_include_directories(i2c_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(i2c_test PRIVATE
    hal
    utils
    pico_stdlib
    FreeRTOS-Kernel-Heap4
)

pico_enable_stdio_usb(i2c_test 1)
pico_enable_stdio_uart(i2c_test 0)

pico_add_extra_outputs(i2c_test)