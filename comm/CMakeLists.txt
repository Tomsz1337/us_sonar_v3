add_library(comm STATIC
    udp_transport.cpp
    comm_service.cpp

)

target_include_directories(comm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(comm PUBLIC
    FreeRTOS-Kernel
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
)