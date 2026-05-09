#pragma once
#include <stdint.h>

#define TFT_WIDTH   240
#define TFT_HEIGHT  320

#define ILI9341_BLACK   0x0000
#define ILI9341_WHITE   0xFFFF
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_BLUE    0x001F
#define ILI9341_YELLOW  0xFFE0
#define ILI9341_CYAN    0x07FF

void ili9341_init(void);
void ili9341_fill_screen(uint16_t color);
void ili9341_fill_rect(int x, int y, int w, int h, uint16_t color);

void ili9341_draw_rgb565_buffer(int x, int y, int w, int h, const uint16_t *buffer);

void ili9341_draw_char(int x, int y, char c, uint16_t color, uint16_t bg, int scale);
void ili9341_draw_string(int x, int y, const char *str, uint16_t color, uint16_t bg, int scale);
