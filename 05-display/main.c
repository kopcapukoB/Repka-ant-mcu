#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "mem-wmem-task/mem-wmem-task.h"
#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

// Пины для подключения дисплея ILI9341
#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9

// Контекст драйвера дисплея
static ili9341_display_t ili9341_display = {0};

// Платформозависимые реализации функций HAL

void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
    spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
    gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
    gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
    gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

// Callback функции команд

void version_callback(const char* args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args)
{
    led_task_state_set(LED_STATE_ON);
}

void led_off_callback(const char* args)
{
    led_task_state_set(LED_STATE_OFF);
}

void led_blink_callback(const char* args)
{
    led_task_state_set(LED_STATE_BLINK);
}

void led_blink_set_period_ms_callback(const char* args)
{
    uint period_ms = 0;
    sscanf(args, "%u", &period_ms);
    if(period_ms == 0)
    {
        printf("Error: period_ms is zero\n");
        return;
    }
    led_task_set_blink_period_ms(period_ms);
}

void disp_screen_callback(const char* args)
{
    uint32_t c = 0;
    int result = sscanf(args, "%x", &c);
    
    uint16_t color = COLOR_BLACK;
    
    if (result == 1)
    {
        color = RGB888_2_RGB565(c);
    }
    
    ili9341_fill_screen(&ili9341_display, color);
    
    if (result == 1)
    {
        printf("Screen filled with color: 0x%06lX\n", c);
    }
    else
    {
        printf("Screen cleared (black)\n");
    }
}

void disp_px_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: disp_px <x> <y> <hex_color>\n");
        return;
    }
    
    uint32_t x = 0, y = 0, c = 0;
    int result = sscanf(args, "%u %u %x", &x, &y, &c);
    
    if (result < 2)
    {
        printf("error: need at least x and y coordinates\n");
        return;
    }
    
    uint16_t color = COLOR_WHITE;
    
    if (result == 3)
    {
        color = RGB888_2_RGB565(c);
    }
    
    if (x >= ili9341_display.width || y >= ili9341_display.height)
    {
        printf("error: coordinates out of bounds (max: %ux%u)\n", 
               ili9341_display.width - 1, ili9341_display.height - 1);
        return;
    }
    
    ili9341_draw_pixel(&ili9341_display, (uint16_t)x, (uint16_t)y, color);
    
    printf("Pixel drawn at (%u, %u) with color: 0x%06lX\n", x, y, c);
}

void disp_line_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: disp_line <x0> <y0> <x1> <y1> <hex_color>\n");
        return;
    }
    
    uint32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0, c = 0;
    int result = sscanf(args, "%u %u %u %u %x", &x0, &y0, &x1, &y1, &c);
    
    if (result < 4)
    {
        printf("error: need at least 4 coordinates\n");
        return;
    }
    
    uint16_t color = COLOR_WHITE;
    
    if (result == 5)
    {
        color = RGB888_2_RGB565(c);
    }
    
    ili9341_draw_line(&ili9341_display, (uint16_t)x0, (uint16_t)y0, (uint16_t)x1, (uint16_t)y1, color);
    
    printf("Line drawn from (%u,%u) to (%u,%u) with color: 0x%06lX\n", x0, y0, x1, y1, c);
}

void disp_rect_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: disp_rect <x> <y> <width> <height> <hex_color>\n");
        return;
    }
    
    uint32_t x = 0, y = 0, w = 0, h = 0, c = 0;
    int result = sscanf(args, "%u %u %u %u %x", &x, &y, &w, &h, &c);
    
    if (result < 4)
    {
        printf("error: need at least x, y, width and height\n");
        return;
    }
    
    uint16_t color = COLOR_WHITE;
    
    if (result == 5)
    {
        color = RGB888_2_RGB565(c);
    }
    
    ili9341_draw_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
    
    printf("Rectangle drawn at (%u,%u) %ux%u with color: 0x%06lX\n", x, y, w, h, c);
}

void disp_frect_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: disp_frect <x> <y> <width> <height> <hex_color>\n");
        return;
    }
    
    uint32_t x = 0, y = 0, w = 0, h = 0, c = 0;
    int result = sscanf(args, "%u %u %u %u %x", &x, &y, &w, &h, &c);
    
    if (result < 4)
    {
        printf("error: need at least x, y, width and height\n");
        return;
    }
    
    uint16_t color = COLOR_WHITE;
    
    if (result == 5)
    {
        color = RGB888_2_RGB565(c);
    }
    
    ili9341_draw_filled_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
    
    printf("Filled rectangle drawn at (%u,%u) %ux%u with color: 0x%06lX\n", x, y, w, h, c);
}

void disp_text_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: disp_text <x> <y> <text_color> <bg_color> <text>\n");
        printf("  example: disp_text 10 100 FFFFFF 000000 Hello World!\n");
        return;
    }
    
    uint32_t x = 0, y = 0, text_c = 0, bg_c = 0;
    char text[256] = {0};
    
    // Считываем координаты и цвета
    int result = sscanf(args, "%u %u %x %x %[^\n]s", &x, &y, &text_c, &bg_c, text);
    
    if (result < 4)
    {
        printf("error: need at least x, y, text_color and bg_color\n");
        printf("usage: disp_text <x> <y> <text_color> <bg_color> <text>\n");
        return;
    }
    
    uint16_t text_color = RGB888_2_RGB565(text_c);
    uint16_t bg_color = RGB888_2_RGB565(bg_c);
    
    if (result == 5 && strlen(text) > 0)
    {
        // Есть текст для отображения
        ili9341_draw_text(&ili9341_display, (uint16_t)x, (uint16_t)y, text, &jetbrains_font, text_color, bg_color);
        printf("Text '%s' drawn at (%u, %u) with text color: 0x%06lX, bg color: 0x%06lX\n", 
               text, x, y, text_c, bg_c);
    }
    else
    {
        // Только очищаем область цветом фона
        ili9341_draw_filled_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, 100, 16, bg_color);
        printf("Area cleared at (%u, %u) with bg color: 0x%06lX\n", x, y, bg_c);
    }
}

void help_callback(const char* args);

void mem_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: mem <hex_address>\n");
        return;
    }
    uint32_t address = (uint32_t)strtoul(args, NULL, 16);
    mem(address);
}

void wmem_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: wmem <hex_addr> <hex_value>\n");
        return;
    }

    char* end_ptr;
    uint32_t addr  = (uint32_t)strtoul(args,    &end_ptr, 16);
    uint32_t value = (uint32_t)strtoul(end_ptr, NULL,     16);
    wmem(addr, value);
}

api_t device_api[] =
{
    {"version",     version_callback,              "get device name and firmware version"},
    {"on",          led_on_callback,               "turn LED on"},
    {"off",         led_off_callback,              "turn LED off"},
    {"blink",       led_blink_callback,            "start LED blinking"},
    {"set_period",  led_blink_set_period_ms_callback, "setting period of LED blinking"},
    {"help",        help_callback,                 "get help about all available commands"},
    {"mem",         mem_callback,                  "get value from address"},
    {"wmem",        wmem_callback,                 "write memory: wmem <hex_addr> <hex_value>"},
    {"disp_screen", disp_screen_callback,          "fill screen: disp_screen <hex_color>"},
    {"disp_px",     disp_px_callback,              "draw pixel: disp_px <x> <y> <hex_color>"},
    {"disp_line",   disp_line_callback,            "draw line: disp_line <x0> <y0> <x1> <y1> <hex_color>"},
    {"disp_rect",   disp_rect_callback,            "draw rect: disp_rect <x> <y> <w> <h> <hex_color>"},
    {"disp_frect",  disp_frect_callback,           "draw filled rect: disp_frect <x> <y> <w> <h> <hex_color>"},
    {"disp_text",   disp_text_callback,            "draw text: disp_text <x> <y> <text_c> <bg_c> <text>"},
    {NULL, NULL, NULL},
};

void help_callback(const char* args)
{
    printf("Available commands:\n");
    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("  %-12s - %s\n", device_api[i].command_name, device_api[i].command_help);
    }
}

int main()
{
    // Инициализация stdio для работы COM порта
    stdio_init_all();
    
    // Инициализация задач
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    // Инициализация SPI на максимальную скорость 62.5 МГц
    spi_init(spi0, 62500000);
    
    // Настройка пинов SPI
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    
    // Инициализация пинов управления дисплеем как GPIO выходы
    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RESET);
    
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);
    
    // Установка начальных уровней
    gpio_put(ILI9341_PIN_CS, 1);    // CS в лог. 1
    gpio_put(ILI9341_PIN_DC, 0);    // DC в лог. 0
    gpio_put(ILI9341_PIN_RESET, 0); // RESET в лог. 0

    // Создание и заполнение структуры HAL
    ili9341_hal_t ili9341_hal = {0};
    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;

    // Инициализация драйвера дисплея
    ili9341_init(&ili9341_display, &ili9341_hal);
    
    // Установка ориентации дисплея 90 градусов
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);

    // Заполняем экран черным фоном
    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);

    printf("Device initialized. Display ready.\n");
    printf("Drawing commands available. Use 'help' for list.\n");
    
    while(1)
    {
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
    }
}