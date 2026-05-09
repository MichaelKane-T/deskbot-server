#!/bin/bash
set -e

mkdir -p src

cat > src/ili9341.h <<'EOL'
#pragma once
#include <stdint.h>

#define TFT_WIDTH   320
#define TFT_HEIGHT  240

#define ILI9341_BLACK   0x0000
#define ILI9341_WHITE   0xFFFF
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_BLUE    0x001F
#define ILI9341_CYAN    0x07FF

void ili9341_init();
void ili9341_fill_screen(uint16_t color);
void ili9341_fill_rect(int x, int y, int w, int h, uint16_t color);
EOL

cat > src/ili9341.cpp <<'EOL'
#include "ili9341.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define TFT_SPI   spi0
#define PIN_MISO  16
#define PIN_CS    17
#define PIN_SCK   18
#define PIN_MOSI  19
#define PIN_DC    20
#define PIN_RST   21
#define PIN_LITE  22

static void cs_select() { gpio_put(PIN_CS, 0); }
static void cs_deselect() { gpio_put(PIN_CS, 1); }
static void dc_command() { gpio_put(PIN_DC, 0); }
static void dc_data() { gpio_put(PIN_DC, 1); }

static void write_cmd(uint8_t cmd) {
    cs_select();
    dc_command();
    spi_write_blocking(TFT_SPI, &cmd, 1);
    cs_deselect();
}

static void write_data(const uint8_t *data, size_t len) {
    cs_select();
    dc_data();
    spi_write_blocking(TFT_SPI, data, len);
    cs_deselect();
}

static void write_data_u8(uint8_t data) {
    write_data(&data, 1);
}

static void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];

    write_cmd(0x2A);
    data[0] = x0 >> 8; data[1] = x0;
    data[2] = x1 >> 8; data[3] = x1;
    write_data(data, 4);

    write_cmd(0x2B);
    data[0] = y0 >> 8; data[1] = y0;
    data[2] = y1 >> 8; data[3] = y1;
    write_data(data, 4);

    write_cmd(0x2C);
}

void ili9341_init() {
    spi_init(TFT_SPI, 40000000);

    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);   gpio_set_dir(PIN_CS, GPIO_OUT);   gpio_put(PIN_CS, 1);
    gpio_init(PIN_DC);   gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_init(PIN_RST);  gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_init(PIN_LITE); gpio_set_dir(PIN_LITE, GPIO_OUT); gpio_put(PIN_LITE, 1);

    gpio_put(PIN_RST, 1); sleep_ms(50);
    gpio_put(PIN_RST, 0); sleep_ms(50);
    gpio_put(PIN_RST, 1); sleep_ms(150);

    write_cmd(0x01);
    sleep_ms(150);

    write_cmd(0x28);

    write_cmd(0x36);
    write_data_u8(0x28);

    write_cmd(0x3A);
    write_data_u8(0x55);

    write_cmd(0x11);
    sleep_ms(150);

    write_cmd(0x29);
    sleep_ms(50);

    ili9341_fill_screen(ILI9341_BLACK);
}

void ili9341_fill_rect(int x, int y, int w, int h, uint16_t color) {
    if (x < 0 || y < 0 || x >= TFT_WIDTH || y >= TFT_HEIGHT) return;
    if (x + w > TFT_WIDTH) w = TFT_WIDTH - x;
    if (y + h > TFT_HEIGHT) h = TFT_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    set_addr_window(x, y, x + w - 1, y + h - 1);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint8_t buffer[128];

    for (int i = 0; i < 128; i += 2) {
        buffer[i] = hi;
        buffer[i + 1] = lo;
    }

    int pixels = w * h;

    cs_select();
    dc_data();

    while (pixels > 0) {
        int chunk = pixels > 64 ? 64 : pixels;
        spi_write_blocking(TFT_SPI, buffer, chunk * 2);
        pixels -= chunk;
    }

    cs_deselect();
}

void ili9341_fill_screen(uint16_t color) {
    ili9341_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}
EOL

cat > src/main.cpp <<'EOL'
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ili9341.h"

extern "C" void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    while (true) {}
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    while (true) {}
}

void heartbeat_task(void *params) {
    while (true) {
        printf("DeskBot heartbeat\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void tft_task(void *params) {
    printf("Initializing TFT...\n");
    ili9341_init();
    printf("TFT initialized\n");

    while (true) {
        ili9341_fill_screen(ILI9341_BLUE);
        vTaskDelay(pdMS_TO_TICKS(500));

        ili9341_fill_screen(ILI9341_RED);
        vTaskDelay(pdMS_TO_TICKS(500));

        ili9341_fill_screen(ILI9341_GREEN);
        vTaskDelay(pdMS_TO_TICKS(500));

        ili9341_fill_screen(ILI9341_BLACK);
        ili9341_fill_rect(70, 60, 60, 60, ILI9341_CYAN);
        ili9341_fill_rect(190, 60, 60, 60, ILI9341_CYAN);
        ili9341_fill_rect(110, 170, 100, 16, ILI9341_WHITE);

        vTaskDelay(pdMS_TO_TICKS(1000));

        ili9341_fill_rect(70, 60, 60, 60, ILI9341_BLACK);
        ili9341_fill_rect(190, 60, 60, 60, ILI9341_BLACK);
        ili9341_fill_rect(70, 88, 60, 8, ILI9341_CYAN);
        ili9341_fill_rect(190, 88, 60, 8, ILI9341_CYAN);

        vTaskDelay(pdMS_TO_TICKS(180));
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    printf("Starting Pico DeskBot...\n");

    xTaskCreate(heartbeat_task, "heartbeat", 512, nullptr, 1, nullptr);
    xTaskCreate(tft_task, "tft", 2048, nullptr, 2, nullptr);

    vTaskStartScheduler();

    while (true) {}
}
EOL

echo "Created source files:"
ls -l src
