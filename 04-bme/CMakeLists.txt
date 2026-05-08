cmake_minimum_required(VERSION 3.15)
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(04_bme)

pico_sdk_init()

# Подключаем библиотеки из общей папки libs
add_subdirectory(../libs/protocol protocol_build)
add_subdirectory(../libs/bme280-driver bme280_build)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task/stdio-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/led-task/led-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mem-wmem-task/mem-wmem-task.c
)

target_link_libraries(${PROJECT_NAME}
    protocol
    bme280_driver
    pico_stdlib
    hardware_i2c
    hardware_gpio
)

pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)