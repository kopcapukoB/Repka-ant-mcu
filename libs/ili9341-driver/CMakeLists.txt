# Находим все исходные файлы в папке src
file(GLOB ILI9341_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

add_library(ili9341 STATIC
    ${ILI9341_SOURCES}
)

target_include_directories(ili9341
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ili9341
    PUBLIC
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_dma
    hardware_pwm
)