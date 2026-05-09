#pragma once

void touch_init(void);

int touch_read_x(void);
int touch_read_y(void);

void touch_get_raw(int *x, int *y);
int touch_is_pressed(int raw_x, int raw_y);
void touch_get_screen(int *x, int *y, int *pressed);

void touch_task(void *p);
