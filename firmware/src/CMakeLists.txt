add_library(deskbot_components
    components/display/ili9341.c
    components/ui/deskbot_ui.c
    components/app/deskbot_state.c
    components/pomodoro/pomodoro.c
    components/touch/touch.c
    components/cloud/cloud_sync.c 
)

target_include_directories(deskbot_components PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/components/display
    ${CMAKE_CURRENT_LIST_DIR}/components/ui
    ${CMAKE_CURRENT_LIST_DIR}/components/app
    ${CMAKE_CURRENT_LIST_DIR}/components/pomodoro
    ${CMAKE_CURRENT_LIST_DIR}/components/touch
    ${CMAKE_CURRENT_LIST_DIR}/components/cloud
)

target_link_libraries(deskbot_components PUBLIC
    pico_stdlib
    hardware_spi
    hardware_adc
    pico_cyw43_arch_lwip_threadsafe_background
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)
