#include "bme280-task.h"
#include "bme280-driver.h"
#include "pico/stdlib.h"

static bme280_data_t current_data = {0};
static uint32_t measure_period_ms = 1000;
static uint32_t last_measure_time = 0;
static bool data_valid = false;

void bme280_task_init(void)
{
    current_data.temperature = 0.0f;
    current_data.pressure = 0.0f;
    current_data.humidity = 0.0f;
    current_data.timestamp = 0;
}

bool bme280_task_get_data(bme280_data_t* data)
{
    if (data && data_valid) {
        *data = current_data;
        return true;
    }
    return false;
}

void bme280_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms > 0) {
        measure_period_ms = period_ms;
    }
}

uint32_t bme280_task_get_period_ms(void)
{
    return measure_period_ms;
}

void bme280_task_handle(void)
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_measure_time >= measure_period_ms) {
        last_measure_time = now;
        
        current_data.temperature = bme280_read_temperature();
        current_data.pressure = bme280_read_pressure();
        current_data.humidity = bme280_read_humidity();
        current_data.timestamp = now;
        data_valid = true;
    }
}