#ifndef BME280_TASK_H
#define BME280_TASK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float temperature;
    float pressure;
    float humidity;
    uint32_t timestamp;
} bme280_data_t;

void bme280_task_init(void);
bool bme280_task_get_data(bme280_data_t* data);
void bme280_task_set_period_ms(uint32_t period_ms);
uint32_t bme280_task_get_period_ms(void);
void bme280_task_handle(void);

#endif