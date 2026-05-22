#include "display-task.h"
#include "ili9341-display.h"
#include "ili9341-font.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Пины дисплея
#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9

static ili9341_display_t display = {0};

static void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
    spi_write_blocking(spi0, data, size);
}

static void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
    spi_read_blocking(spi0, 0, buffer, length);
}

static void rp2040_gpio_cs_write(bool level)
{
    gpio_put(ILI9341_PIN_CS, level);
}

static void rp2040_gpio_dc_write(bool level)
{
    gpio_put(ILI9341_PIN_DC, level);
}

static void rp2040_gpio_reset_write(bool level)
{
    gpio_put(ILI9341_PIN_RESET, level);
}

static void rp2040_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

void display_task_init(void)
{
    spi_init(spi0, 62500000);
    
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    
    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RESET);
    
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);
    
    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_RESET, 0);

    ili9341_hal_t hal = {0};
    hal.spi_write = rp2040_spi_write;
    hal.spi_read = rp2040_spi_read;
    hal.gpio_cs_write = rp2040_gpio_cs_write;
    hal.gpio_dc_write = rp2040_gpio_dc_write;
    hal.gpio_reset_write = rp2040_gpio_reset_write;
    hal.delay_ms = rp2040_delay_ms;

    ili9341_init(&display, &hal);
    ili9341_set_rotation(&display, ILI9341_ROTATION_90);
    ili9341_fill_screen(&display, COLOR_BLACK);
}

ili9341_display_t* display_task_get_display(void)
{
    return &display;
}

void display_task_clear(uint16_t color)
{
    ili9341_fill_screen(&display, color);
}

void display_task_draw_text(uint16_t x, uint16_t y, const char* text, uint16_t text_color, uint16_t bg_color)
{
    ili9341_draw_text(&display, x, y, text, &jetbrains_font, text_color, bg_color);
}

void display_task_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ili9341_draw_rect(&display, x, y, w, h, color);
}

void display_task_draw_filled_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ili9341_draw_filled_rect(&display, x, y, w, h, color);
}

void display_task_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    ili9341_draw_line(&display, x0, y0, x1, y1, color);
}