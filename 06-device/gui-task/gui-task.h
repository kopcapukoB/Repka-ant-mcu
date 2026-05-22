#ifndef GUI_TASK_H
#define GUI_TASK_H

#include <stdint.h>

#define GUI_HISTORY_SIZE 60

void gui_task_init(void);
void gui_task_update(void);
void gui_task_draw(void);

#endif