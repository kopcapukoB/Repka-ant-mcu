#include "mem-wmem-task.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdint.h"
#include "stdio.h"

void mem(uint32_t addr)
{
    uint32_t* ptr    = (uint32_t*)addr; 
    uint32_t  value  = *ptr;

    printf("mem[0x%08X] = 0x%08X\n", addr, value);
}

void wmem(uint32_t addr, uint32_t value)
{
    *(volatile uint32_t*)addr = value;

    printf("wmem[0x%08X] <- 0x%08X done\n", addr, value);
}