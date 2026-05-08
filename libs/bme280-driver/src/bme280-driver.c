#include "bme280-driver.h"

static bme280_ctx_t bme280_ctx = {0};

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write)
{
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;
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