cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
set(PICO_SDK_PATH "/opt/pico-sdk")

set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/config" CACHE STRING "" FORCE)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(config/FreeRTOS_Kernel_import.cmake)

project(us_sonar_v3 C CXX ASM)

add_compile_definitions(
    PICO_FREERTOS=1
    PICO_CYW43_ARCH_FREERTOS=1
    NO_SYS=0
    #FREE_RTOS_KERNEL_SMP=0

)

pico_sdk_init()

add_subdirectory(utils)
# add_subdirectory(hal)
# add_subdirectory(drivers)
add_subdirectory(comm)
# add_subdirectory(segger_rtt)

add_executable(firmware
    src/main.cpp
    system/system_init.cpp
    tasks/comm_task.cpp
    tasks/control_task.cpp
)

pico_enable_stdio_usb(firmware 0)
pico_enable_stdio_uart(firmware 0)
pico_enable_stdio_rtt(firmware 0)

target_include_directories(firmware PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config
)
target_compile_options(firmware PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>
)
target_link_libraries(firmware PRIVATE
    pico_stdlib
    #pico_multicore
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    utils
    #hal
    #drivers
    comm
    #systemview
)

pico_add_extra_outputs(firmware)