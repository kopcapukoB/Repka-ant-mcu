#ifndef ADC_TASK_H
#define ADC_TASK_H

#include "pico/stdlib.h"

// Конфигурация пинов
#define ADC_TASK_GPIO_NUMBER      26
#define ADC_TASK_ADC_CHANNEL_NUMBER 0   // GPIO26 = ADC0

// Состояния задачи
typedef enum {
    ADC_TASK_STATE_IDLE = 0,
    ADC_TASK_STATE_RUN
} adc_task_state_t;

// Публичные функции
void adc_task_init(void);
float adc_task_get_voltage(void);
float adc_task_get_temp(void);
void adc_task_set_state(adc_task_state_t state);
void adc_task_handle(void);

#endif // ADC_TASK_H