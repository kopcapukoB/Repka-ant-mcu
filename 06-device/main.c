#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "mem-wmem-task/mem-wmem-task.h"
#include "bme280-driver.h"
#include "bme280-task.h"
#include "display-task.h"
#include "gui-task.h"

#define DEVICE_NAME "WeatherStation"
#define DEVICE_VRSN "v1.0.0"
#define BME280_I2C_ADDR 0x76

// I2C функции для BME280
void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    i2c_read_timeout_us(i2c1, BME280_I2C_ADDR, buffer, length, false, 500000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    i2c_write_timeout_us(i2c1, BME280_I2C_ADDR, data, size, false, 500000);
}

// Callback функции команд
void version_callback(const char* args)
{
    printf("%s %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void help_callback(const char* args);

void temp_callback(const char* args)
{
    bme280_data_t data;
    if (bme280_task_get_data(&data)) {
        printf("Temperature: %.2f C\n", data.temperature);
    } else {
        printf("No data available\n");
    }
}

void pres_callback(const char* args)
{
    bme280_data_t data;
    if (bme280_task_get_data(&data)) {
        printf("Pressure: %.2f hPa\n", data.pressure);
    } else {
        printf("No data available\n");
    }
}

void hum_callback(const char* args)
{
    bme280_data_t data;
    if (bme280_task_get_data(&data)) {
        printf("Humidity: %.2f %%RH\n", data.humidity);
    } else {
        printf("No data available\n");
    }
}

void all_callback(const char* args)
{
    bme280_data_t data;
    if (bme280_task_get_data(&data)) {
        printf("T:%.1f P:%.1f H:%.1f\n", data.temperature, data.pressure, data.humidity);
    } else {
        printf("No data available\n");
    }
}

void set_period_callback(const char* args)
{
    uint32_t period = 0;
    if (sscanf(args, "%u", &period) == 1 && period > 0) {
        bme280_task_set_period_ms(period);
        printf("Measurement period set to %u ms\n", period);
    } else {
        printf("Usage: set_period <ms>\n");
    }
}

void get_period_callback(const char* args)
{
    printf("Measurement period: %u ms\n", bme280_task_get_period_ms());
}

api_t device_api[] =
{
    {"version",     version_callback,      "get device name and version"},
    {"help",        help_callback,         "get help about commands"},
    {"temp",        temp_callback,         "get temperature"},
    {"pres",        pres_callback,         "get pressure"},
    {"hum",         hum_callback,          "get humidity"},
    {"all",         all_callback,          "get all measurements"},
    {"set_period",  set_period_callback,   "set measurement period (ms)"},
    {"get_period",  get_period_callback,   "get current measurement period"},
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
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    // Инициализация I2C для BME280
    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    // Инициализация BME280
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    bme280_task_init();
    
    // Инициализация дисплея
    display_task_init();
    
    // Инициализация GUI
    gui_task_init();
    gui_task_draw();

    printf("%s %s started\n", DEVICE_NAME, DEVICE_VRSN);
    
    uint32_t last_gui_update = 0;
    
    while(1)
    {
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
        bme280_task_handle();
        
        // Обновляем GUI каждую секунду
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_gui_update >= 1000) {
            last_gui_update = now;
            gui_task_update();
            gui_task_draw();
        }
    }
}