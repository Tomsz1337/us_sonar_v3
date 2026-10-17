add_library(utils
    LockGuard.cpp
    hooks.cpp
)

target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(utils PUBLIC
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)