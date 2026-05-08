#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "mem-wmem-task/mem-wmem-task.h"

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

    if(period_ms==0)
    {
        printf("Error: perios_ms is zero");
        return;
    }

    led_task_set_blink_period_ms(period_ms);
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
    led_task_state_set(LED_STATE_BLINK);
}

api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "start LED blinking"},
    {"set_period", led_blink_set_period_ms_callback, "setting period of LED blinking"},
    {"help", help_callback, "get help about all avaliable commands"},
    {"mem", mem_callback, "get value from adress"},
    {"wmem",    wmem_callback,     "write memory: wmem <hex_addr> <hex_value>"},
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
    //stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    
    while(1)
    {
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
    }
}