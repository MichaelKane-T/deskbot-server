cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

project(pico_deskbot C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

add_subdirectory(src)

add_executable(pico_deskbot
    src/main.c
)

target_include_directories(pico_deskbot PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
)


target_link_libraries(pico_deskbot
    pico_stdlib
    hardware_spi
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    deskbot_components
)

pico_enable_stdio_usb(pico_deskbot 1)
pico_enable_stdio_uart(pico_deskbot 0)

pico_add_extra_outputs(pico_deskbot)
