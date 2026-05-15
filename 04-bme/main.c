#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "bme280-driver.h"
#include "led-task/led-task.h"
#include "mem-wmem-task/mem-wmem-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"
#define BME280_I2C_ADDR 0x76

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    int result = i2c_read_timeout_us(i2c1, BME280_I2C_ADDR, buffer, length, false, 500000);
    if (result < 0) {
        printf("I2C read error: %d\n", result);
    }
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    int result = i2c_write_timeout_us(i2c1, BME280_I2C_ADDR, data, size, false, 500000);
    if (result < 0) {
        printf("I2C write error: %d\n", result);
    }
}

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

void read_regs_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: read_regs <hex_addr> <count>\n");
        return;
    }

    char* end_ptr;
    uint32_t addr = (uint32_t)strtoul(args, &end_ptr, 16);
    uint32_t N = (uint32_t)strtoul(end_ptr, NULL, 16);

    if (addr > 0xFF)
    {
        printf("error: addr must be <= 0xFF\n");
        return;
    }

    if (N > 0xFF || N == 0)
    {
        printf("error: count must be 1..0xFF\n");
        return;
    }

    if (addr + N > 0x100)
    {
        printf("error: addr + count must be <= 0x100\n");
        return;
    }

    uint8_t buffer[256] = {0};
    bme280_read_regs((uint8_t)addr, buffer, (uint8_t)N);

    printf("BME280 registers:\n");
    for (uint32_t i = 0; i < N; i++)
    {
        printf("  bme280 register [0x%02X] = 0x%02X\n", (uint8_t)(addr + i), buffer[i]);
    }
}

void write_reg_callback(const char* args)
{
    if (args == NULL || args[0] == '\0')
    {
        printf("error: usage: write_reg <hex_addr> <hex_value>\n");
        return;
    }

    char* end_ptr;
    uint32_t addr = (uint32_t)strtoul(args, &end_ptr, 16);
    uint32_t value = (uint32_t)strtoul(end_ptr, NULL, 16);

    if (addr > 0xFF)
    {
        printf("error: addr must be <= 0xFF\n");
        return;
    }

    if (value > 0xFF)
    {
        printf("error: value must be <= 0xFF\n");
        return;
    }

    bme280_write_reg((uint8_t)addr, (uint8_t)value);
    printf("BME280: wrote 0x%02X to register 0x%02X\n", (uint8_t)value, (uint8_t)addr);
}

void temp_raw_callback(const char* args)
{
    uint32_t temp = bme280_read_temp_raw();
    printf("Temperature raw: %lu\n", temp);
}

void pres_raw_callback(const char* args)
{
    uint32_t pres = bme280_read_pres_raw();
    printf("Pressure raw: %lu\n", pres);
}

void hum_raw_callback(const char* args)
{
    uint16_t hum = bme280_read_hum_raw();
    printf("Humidity raw: %u\n", hum);
}

void temp_callback(const char* args)
{
    float temp = bme280_read_temperature();
    printf("Temperature: %.2f °C\n", temp);
}

void pres_callback(const char* args)
{
    float pres = bme280_read_pressure();
    printf("Pressure: %.2f hPa\n", pres);
}

void hum_callback(const char* args)
{
    float hum = bme280_read_humidity();
    printf("Humidity: %.2f %%RH\n", hum);
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
    {"read_regs",   read_regs_callback,            "read BME280 registers: read_regs <addr> <count>"},
    {"write_reg",   write_reg_callback,            "write BME280 register: write_reg <addr> <value>"},
    {"temp_raw",    temp_raw_callback,             "get raw temperature value"},
    {"pres_raw",    pres_raw_callback,             "get raw pressure value"},
    {"hum_raw",     hum_raw_callback,              "get raw humidity value"},
    {"temp",        temp_callback,                 "get temperature in °C"},
    {"pres",        pres_callback,                 "get pressure in hPa"},
    {"hum",         hum_callback,                  "get humidity in %RH"},
    {NULL, NULL, NULL},
};

void help_callback(const char* args)
{
    printf("Available commands:\n");
    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("  %-10s — %s\n", device_api[i].command_name, device_api[i].command_help);
    }
}

int main()
{
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    i2c_init(i2c1, 100000);
    
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    
    gpio_pull_up(14);
    gpio_pull_up(15);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    printf("Device initialized. BME280 address: 0x%02X\n", BME280_I2C_ADDR);
    
    while(1)
    {
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
    }
}