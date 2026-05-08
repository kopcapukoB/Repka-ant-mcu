#include "adc-task.h"
#include "hardware/adc.h"
#include "stdio.h"
#include "pico/stdlib.h"

// Период измерений (100 000 мкс = 0.1 секунды = 10 Гц)
#define ADC_TASK_MEAS_PERIOD_US 100000

// Приватные переменные
static adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
static uint64_t time_stamp = 0;

void adc_task_init()
{
    adc_init();
    adc_gpio_init(ADC_TASK_GPIO_NUMBER);
    adc_set_temp_sensor_enabled(true);
}

float adc_task_get_voltage()
{
    adc_select_input(ADC_TASK_ADC_CHANNEL_NUMBER);
    uint16_t voltage_counts = adc_read();
    float voltage_V = voltage_counts * 3.3f / 4096.0f;
    return voltage_V;
}

float adc_task_get_temp()
{
    adc_select_input(4);  // встроенный датчик температуры
    uint16_t voltage_counts = adc_read();
    float voltage_V = voltage_counts * 3.3f / 4096.0f;
    float temp_C = 27.0f - (voltage_V - 0.706f) / 0.001721f;
    return temp_C;
}

void adc_task_set_state(adc_task_state_t state)
{
    adc_state = state;
}

void adc_task_handle()
{
    switch (adc_state)
    {
    case ADC_TASK_STATE_IDLE:
        break;
        
    case ADC_TASK_STATE_RUN:
        if (time_us_64() > time_stamp)
        {
            time_stamp = time_us_64() + ADC_TASK_MEAS_PERIOD_US;
            float temp_C = adc_task_get_temp();
            float voltage_V = adc_task_get_voltage();
            printf("%f %f\n", voltage_V, temp_C);
        }
        break;
        
    default:
        break;
    }
}