cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
set(PICO_SDK_PATH "/opt/pico-sdk")
#set(FREERTOS_KERNEL_PATH "/opt/FreeRTOS-Kernel")

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(config/FreeRTOS_Kernel_import.cmake)

project(us_sonar_v3 C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.cpp
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FREE_RTOS_KERNEL_SMP=1
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    pico_multicore
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(${PROJECT_NAME})