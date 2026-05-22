#include "gui-task.h"
#include "display-task.h"
#include "bme280-task.h"
#include <stdio.h>
#include <string.h>

// Цвета
#define COLOR_BG        COLOR_BLACK
#define COLOR_TEMP      RGB888_2_RGB565(0xFF4444)
#define COLOR_PRESS     RGB888_2_RGB565(0x4444FF)
#define COLOR_HUM       RGB888_2_RGB565(0x44FF44)
#define COLOR_TEXT      0xFFFF
#define COLOR_GRAPH_BG  RGB888_2_RGB565(0x1A1A1A)
#define COLOR_GRID      RGB888_2_RGB565(0x333333)

// История измерений
static float temp_history[GUI_HISTORY_SIZE] = {0};
static float press_history[GUI_HISTORY_SIZE] = {0};
static float hum_history[GUI_HISTORY_SIZE] = {0};
static int history_index = 0;
static int history_count = 0;

// Границы графиков
#define GRAPH_X 5
#define GRAPH_Y 140
#define GRAPH_W 310
#define GRAPH_H 95

static void draw_header(void)
{
    display_task_draw_filled_rect(0, 0, 320, 25, RGB888_2_RGB565(0x1A1A2E));
    display_task_draw_text(5, 5, "Weather Station", COLOR_TEXT, RGB888_2_RGB565(0x1A1A2E));
}

static void draw_measurements(void)
{
    bme280_data_t data;
    char buffer[32];
    
    if (bme280_task_get_data(&data)) {
        // Температура
        display_task_draw_filled_rect(5, 30, 310, 30, RGB888_2_RGB565(0x2A1A1A));
        sprintf(buffer, "Temp: %.1f C", data.temperature);
        display_task_draw_text(10, 37, buffer, COLOR_TEMP, RGB888_2_RGB565(0x2A1A1A));
        
        // Давление
        display_task_draw_filled_rect(5, 65, 310, 30, RGB888_2_RGB565(0x1A1A2A));
        sprintf(buffer, "Press: %.1f hPa", data.pressure);
        display_task_draw_text(10, 72, buffer, COLOR_PRESS, RGB888_2_RGB565(0x1A1A2A));
        
        // Влажность
        display_task_draw_filled_rect(5, 100, 310, 30, RGB888_2_RGB565(0x1A2A1A));
        sprintf(buffer, "Hum: %.1f %%RH", data.humidity);
        display_task_draw_text(10, 107, buffer, COLOR_HUM, RGB888_2_RGB565(0x1A2A1A));
    }
}

static void update_history(void)
{
    bme280_data_t data;
    
    if (bme280_task_get_data(&data)) {
        temp_history[history_index] = data.temperature;
        press_history[history_index] = data.pressure;
        hum_history[history_index] = data.humidity;
        
        history_index = (history_index + 1) % GUI_HISTORY_SIZE;
        if (history_count < GUI_HISTORY_SIZE) {
            history_count++;
        }
    }
}

static void draw_graphs(void)
{
    if (history_count < 2) return;
    
    // Фон графиков
    display_task_draw_filled_rect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, COLOR_GRAPH_BG);
    
    // Рамка
    display_task_draw_rect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, COLOR_GRID);
    
    // Находим мин/макс для каждого параметра
    float temp_min = temp_history[0], temp_max = temp_history[0];
    float press_min = press_history[0], press_max = press_history[0];
    float hum_min = hum_history[0], hum_max = hum_history[0];
    
    for (int i = 0; i < history_count; i++) {
        if (temp_history[i] < temp_min) temp_min = temp_history[i];
        if (temp_history[i] > temp_max) temp_max = temp_history[i];
        if (press_history[i] < press_min) press_min = press_history[i];
        if (press_history[i] > press_max) press_max = press_history[i];
        if (hum_history[i] < hum_min) hum_min = hum_history[i];
        if (hum_history[i] > hum_max) hum_max = hum_history[i];
    }
    
    // Добавляем отступы
    float temp_range = temp_max - temp_min;
    float press_range = press_max - press_min;
    float hum_range = hum_max - hum_min;
    
    if (temp_range == 0) temp_range = 1;
    if (press_range == 0) press_range = 1;
    if (hum_range == 0) hum_range = 1;
    
    temp_min -= temp_range * 0.1f;
    temp_max += temp_range * 0.1f;
    press_min -= press_range * 0.1f;
    press_max += press_range * 0.1f;
    hum_min -= hum_range * 0.1f;
    hum_max += hum_range * 0.1f;
    
    temp_range = temp_max - temp_min;
    press_range = press_max - press_min;
    hum_range = hum_max - hum_min;
    
    // Рисуем графики
    for (int i = 1; i < history_count; i++) {
        int idx1 = (history_index - history_count + i - 1 + GUI_HISTORY_SIZE) % GUI_HISTORY_SIZE;
        int idx2 = (history_index - history_count + i + GUI_HISTORY_SIZE) % GUI_HISTORY_SIZE;
        
        int x1 = GRAPH_X + ((i - 1) * GRAPH_W) / (GUI_HISTORY_SIZE - 1);
        int x2 = GRAPH_X + (i * GRAPH_W) / (GUI_HISTORY_SIZE - 1);
        
        // Температура
        int y1 = GRAPH_Y + GRAPH_H - (int)(((temp_history[idx1] - temp_min) / temp_range) * GRAPH_H);
        int y2 = GRAPH_Y + GRAPH_H - (int)(((temp_history[idx2] - temp_min) / temp_range) * GRAPH_H);
        display_task_draw_line(x1, y1, x2, y2, COLOR_TEMP);
        
        // Давление
        y1 = GRAPH_Y + GRAPH_H - (int)(((press_history[idx1] - press_min) / press_range) * GRAPH_H);
        y2 = GRAPH_Y + GRAPH_H - (int)(((press_history[idx2] - press_min) / press_range) * GRAPH_H);
        display_task_draw_line(x1, y1, x2, y2, COLOR_PRESS);
        
        // Влажность
        y1 = GRAPH_Y + GRAPH_H - (int)(((hum_history[idx1] - hum_min) / hum_range) * GRAPH_H);
        y2 = GRAPH_Y + GRAPH_H - (int)(((hum_history[idx2] - hum_min) / hum_range) * GRAPH_H);
        display_task_draw_line(x1, y1, x2, y2, COLOR_HUM);
    }
    
    // Легенда
    display_task_draw_text(5, 140, "T", COLOR_TEMP, COLOR_GRAPH_BG);
    display_task_draw_text(5, 170, "P", COLOR_PRESS, COLOR_GRAPH_BG);
    display_task_draw_text(5, 200, "H", COLOR_HUM, COLOR_GRAPH_BG);
}

void gui_task_init(void)
{
    for (int i = 0; i < GUI_HISTORY_SIZE; i++) {
        temp_history[i] = 0;
        press_history[i] = 0;
        hum_history[i] = 0;
    }
}

void gui_task_update(void)
{
    update_history();
}

void gui_task_draw(void)
{
    draw_header();
    draw_measurements();
    draw_graphs();
}