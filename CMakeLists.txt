# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
cmake_minimum_required(VERSION 3.21)

include(pico_sdk_import.cmake)

include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(pico-projects C CXX ASM)

set(PICO_BOARD pico_w)

pico_sdk_init()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_executable(joystick
   joystick.c
   main.c
   utils/send-data-to-server/send-data.c
   utils/wifi-connection/wifi-connection.c
)

target_include_directories(joystick PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(joystick 1)
pico_enable_stdio_uart(joystick 0)

target_link_libraries(joystick
    pico_stdlib
    hardware_adc
    pico_cyw43_arch_lwip_threadsafe_background  # biblioteca do Wi-Fi
    hardware_gpio 
    pico_multicore 
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4
)

target_include_directories(joystick PRIVATE
    ${PICO_SDK_PATH}/lib/pico_cyw43_driver/include
    ${PICO_SDK_PATH}/lib/pico_cyw43_arch/include
    ${PICO_SDK_PATH}/lib/pico_lwip/include
    utils/html-renderer
    utils/send-data-to-server
    utils/wifi-connection
)




pico_add_extra_outputs(joystick)


