cmake_minimum_required(VERSION 3.15)
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(06_device)

pico_sdk_init()

# Подключаем библиотеки
add_subdirectory(../libs/protocol protocol_build)
add_subdirectory(../libs/bme280-driver bme280_build)
add_subdirectory(../libs/ili9341-driver ili9341_build)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task/stdio-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol-task/protocol-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/led-task/led-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mem-wmem-task/mem-wmem-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bme280-task/bme280-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/display-task/display-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gui-task/gui-task.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol-task
    ${CMAKE_CURRENT_SOURCE_DIR}/led-task
    ${CMAKE_CURRENT_SOURCE_DIR}/mem-wmem-task
    ${CMAKE_CURRENT_SOURCE_DIR}/bme280-task
    ${CMAKE_CURRENT_SOURCE_DIR}/display-task
    ${CMAKE_CURRENT_SOURCE_DIR}/gui-task
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/bme280-driver
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/bme280-driver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ili9341-driver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/protocol/include
)

target_link_libraries(${PROJECT_NAME}
    protocol
    bme280_driver
    ili9341
    pico_stdlib
    hardware_i2c
    hardware_spi
    hardware_gpio
)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})