#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include <stdint.h>
#include "ili9341-driver.h"

void display_task_init(void);
ili9341_display_t* display_task_get_display(void);
void display_task_clear(uint16_t color);
void display_task_draw_text(uint16_t x, uint16_t y, const char* text, uint16_t text_color, uint16_t bg_color);
void display_task_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_task_draw_filled_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_task_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

#endif