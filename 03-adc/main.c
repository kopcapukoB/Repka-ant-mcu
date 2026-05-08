#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "mem-wmem-task/mem-wmem-task.h"
#include "adc-task/adc-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

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
        printf("impossible period %s\n");
        return;
    }
    led_task_set_blink_period_ms(period_ms);
}

// Чтение напряжения с АЦП
void adc_voltage_callback(const char* args)
{
    float voltage = adc_task_get_voltage();
    printf("%.3f\n", voltage);
}

void adc_temp_callback(const char* args)
{
    float temp = adc_task_get_temp();
    printf("%.2f\n", temp);
}

// Запуск непрерывного мониторинга АЦП
void adc_monitor_start_callback(const char* args)
{
    adc_task_set_state(ADC_TASK_STATE_RUN);
    printf("ADC monitoring started\n");
}

// Остановка мониторинга АЦП
void adc_monitor_stop_callback(const char* args)
{
    adc_task_set_state(ADC_TASK_STATE_IDLE);
    printf("ADC monitoring stopped\n");
}

// Показать оба значения сразу
void adc_all_callback(const char* args)
{
    float voltage = adc_task_get_voltage();
    float temp = adc_task_get_temp();
    printf("Voltage: %.3f V | Temperature: %.2f °C\n", voltage, temp);
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
    {"version",     version_callback,                  "get device name and firmware version"},
    {"on",          led_on_callback,                   "turn LED on"},
    {"blink",       led_blink_callback,                "turn LED blink"},
    {"off",         led_off_callback,                  "turn LED off"},
    {"set_period",  led_blink_set_period_ms_callback,  "set LED blink period"},
    {"help",        help_callback,                     "print command description"},
    {"mem",         mem_callback,                      "get value from address"},
    {"wmem",        wmem_callback,                     "write memory: wmem <hex_addr> <hex_value>"},
    {"get_adc",     adc_voltage_callback,              "read ADC voltage"},
    {"get_temp",    adc_temp_callback,                 "read temperature"},
    {"monitor",     adc_monitor_start_callback,        "start ADC monitoring"},
    {"mon_stop",    adc_monitor_stop_callback,         "stop ADC monitoring"},
    {"all",         adc_all_callback,                  "show voltage and temperature"},
    {NULL, NULL, NULL},
};

void help_callback(const char* args)
{
    printf("=== Available Commands ===\n");
    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("  %-12s — %s\n", device_api[i].command_name, device_api[i].command_help);
    }
}

int main()
{
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    adc_task_init();

    while(1)
    {
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
        adc_task_handle();
    }
}