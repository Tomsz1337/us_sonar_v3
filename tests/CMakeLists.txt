add_executable(i2c_test
    i2c_scanner.cpp
)

target_include_directories(i2c_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(i2c_test PRIVATE
    hal
    utils
    drivers
    pico_stdlib
    FreeRTOS-Kernel-Heap4
    systemview
)

