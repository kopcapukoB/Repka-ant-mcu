#include "bme280-driver.h"
#include "../include/bme280-regs.h"
#include <stdio.h>
#include <math.h>

static bme280_ctx_t bme280_ctx = {0};
static int32_t t_fine; // Global temperature fine value for compensation

// Calibration data structure
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_data_t;

static bme280_calib_data_t calib_data;

// Read calibration data from BME280
static void bme280_read_calibration_data(void)
{
    uint8_t calib[26];
    
    // Read temperature calibration data
    bme280_read_regs(0x88, calib, 24);
    
    calib_data.dig_T1 = (uint16_t)((calib[1] << 8) | calib[0]);
    calib_data.dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
    calib_data.dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);
    
    calib_data.dig_P1 = (uint16_t)((calib[7] << 8) | calib[6]);
    calib_data.dig_P2 = (int16_t)((calib[9] << 8) | calib[8]);
    calib_data.dig_P3 = (int16_t)((calib[11] << 8) | calib[10]);
    calib_data.dig_P4 = (int16_t)((calib[13] << 8) | calib[12]);
    calib_data.dig_P5 = (int16_t)((calib[15] << 8) | calib[14]);
    calib_data.dig_P6 = (int16_t)((calib[17] << 8) | calib[16]);
    calib_data.dig_P7 = (int16_t)((calib[19] << 8) | calib[18]);
    calib_data.dig_P8 = (int16_t)((calib[21] << 8) | calib[20]);
    calib_data.dig_P9 = (int16_t)((calib[23] << 8) | calib[22]);
    
    // Read humidity calibration data
    bme280_read_regs(0xA1, calib, 1);
    calib_data.dig_H1 = calib[0];
    
    uint8_t calib_h[7];
    bme280_read_regs(0xE1, calib_h, 7);
    
    calib_data.dig_H2 = (int16_t)((calib_h[1] << 8) | calib_h[0]);
    calib_data.dig_H3 = calib_h[2];
    calib_data.dig_H4 = (int16_t)((calib_h[3] << 4) | (calib_h[4] & 0x0F));
    calib_data.dig_H5 = (int16_t)((calib_h[5] << 4) | ((calib_h[4] >> 4) & 0x0F));
    calib_data.dig_H6 = (int8_t)calib_h[6];
}

// Temperature compensation (returns temperature in 0.01 °C)
static int32_t bme280_compensate_temp(int32_t adc_T)
{
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * 
            ((int32_t)calib_data.dig_T2)) >> 11;
    
    var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) * 
              ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) * 
            ((int32_t)calib_data.dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    
    return T;
}

// Pressure compensation (returns pressure in Pa)
static uint32_t bme280_compensate_press(int32_t adc_P)
{
    int64_t var1, var2, p;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_data.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_data.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_data.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_data.dig_P3) >> 8) + 
           ((var1 * (int64_t)calib_data.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.dig_P1) >> 33;
    
    if (var1 == 0)
    {
        return 0;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_data.dig_P8) * p) >> 19;
    
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
    
    return (uint32_t)p;
}

// Humidity compensation (returns humidity in %RH * 1024)
static uint32_t bme280_compensate_hum(int32_t adc_H)
{
    int32_t v_x1_u32r;
    
    v_x1_u32r = (t_fine - ((int32_t)76800));
    
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib_data.dig_H4) << 20) - 
                    (((int32_t)calib_data.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * 
                 (((((((v_x1_u32r * ((int32_t)calib_data.dig_H6)) >> 10) * 
                      (((v_x1_u32r * ((int32_t)calib_data.dig_H3)) >> 11) + 
                       ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * 
                   ((int32_t)calib_data.dig_H2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
                                ((int32_t)calib_data.dig_H1)) >> 4));
    
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    
    return (uint32_t)(v_x1_u32r >> 12);
}

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write)
{
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;
    
    // Check chip ID
    uint8_t id_reg_buf[1] = {0};
    bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));
    
    if (id_reg_buf[0] != BME280_CHIP_ID) {
        printf("BME280 ID check failed! Expected: 0x%02X, Got: 0x%02X\n", 
               BME280_CHIP_ID, id_reg_buf[0]);
        return;
    } else {
        printf("BME280 ID check passed: 0x%02X\n", id_reg_buf[0]);
    }
    
    // Read calibration data
    bme280_read_calibration_data();
    
    // Configure humidity oversampling
    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = oversampling 1
    bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);
    
    // Configure sensor
    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0 << 0);    // spi3w_en[0:0] = false
    config_reg_value |= (0b000 << 2);  // filter[4:2] = Filter off
    config_reg_value |= (0b001 << 5);  // t_sb[7:5] = 62.5 ms
    bme280_write_reg(BME280_REG_config, config_reg_value);
    
    // Configure measurement mode
    uint8_t ctrl_meas_reg_value = 0;
    ctrl_meas_reg_value |= (0b11 << 0);   // mode[1:0] = Normal mode
    ctrl_meas_reg_value |= (0b001 << 5);  // osrs_t[7:5] = oversampling 1
    ctrl_meas_reg_value |= (0b001 << 2);  // osrs_p[4:2] = oversampling 1
    bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length)
{
    uint8_t data[1] = {start_reg_address};
    
    bme280_ctx.i2c_write(data, sizeof(data));
    bme280_ctx.i2c_read(buffer, length);
}

void bme280_write_reg(uint8_t reg_address, uint8_t value)
{
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

uint32_t bme280_read_temp_raw()
{
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
    uint32_t value = ((uint32_t)read[0] << 12) | ((uint32_t)read[1] << 4) | ((uint32_t)read[2] >> 4);
    return value;
}

uint32_t bme280_read_pres_raw()
{
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_press_msb, read, sizeof(read));
    uint32_t value = ((uint32_t)read[0] << 12) | ((uint32_t)read[1] << 4) | ((uint32_t)read[2] >> 4);
    return value;
}

uint16_t bme280_read_hum_raw()
{
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_hum_msb, read, sizeof(read));
    uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
    return value;
}

float bme280_read_temperature()
{
    int32_t adc_T = (int32_t)bme280_read_temp_raw();
    int32_t temp_100 = bme280_compensate_temp(adc_T);
    return (float)temp_100 / 100.0f; // Convert to °C
}

float bme280_read_pressure()
{
    // First read temperature to update t_fine
    int32_t adc_T = (int32_t)bme280_read_temp_raw();
    bme280_compensate_temp(adc_T);
    
    int32_t adc_P = (int32_t)bme280_read_pres_raw();
    uint32_t pressure = bme280_compensate_press(adc_P);
    return (float)pressure / 256.0f; // Convert to hPa
}

float bme280_read_humidity()
{
    // First read temperature to update t_fine
    int32_t adc_T = (int32_t)bme280_read_temp_raw();
    bme280_compensate_temp(adc_T);
    
    int32_t adc_H = (int32_t)bme280_read_hum_raw();
    uint32_t humidity = bme280_compensate_hum(adc_H);
    return (float)humidity / 1024.0f; // Convert to %RH
}