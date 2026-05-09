#include "touch.h"
#include "deskbot_state.h"

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cloud_sync.h"

#include <stdio.h>

#define TOUCH_XP 26
#define TOUCH_XM 27
#define TOUCH_YP 15
#define TOUCH_YM 28

#define SCREEN_W   240
#define SCREEN_H   320

#define RAW_X_MIN  620
#define RAW_X_MAX  1880
#define RAW_Y_MIN  250
#define RAW_Y_MAX  1450

static int clamp_int(int v, int min, int max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static int map_int(int v, int in_min, int in_max, int out_min, int out_max) {
    return (v - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void pin_digital_out(uint pin, int value) {
    gpio_init(pin);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, value);
}

static void pins_high_z(void) {
    gpio_set_dir(TOUCH_XP, GPIO_IN);
    gpio_set_dir(TOUCH_XM, GPIO_IN);
    gpio_set_dir(TOUCH_YP, GPIO_IN);
    gpio_set_dir(TOUCH_YM, GPIO_IN);
}

void touch_init(void) {
    adc_init();

    gpio_init(TOUCH_XP);
    gpio_init(TOUCH_XM);
    gpio_init(TOUCH_YP);
    gpio_init(TOUCH_YM);

    pins_high_z();
}

int touch_read_x(void) {
    pins_high_z();

    pin_digital_out(TOUCH_XP, 1);
    pin_digital_out(TOUCH_XM, 0);

    adc_gpio_init(TOUCH_YP);
    adc_select_input(2);

    sleep_us(100);
    return adc_read();
}

int touch_read_y(void) {
    pins_high_z();

    pin_digital_out(TOUCH_YP, 1);
    pin_digital_out(TOUCH_YM, 0);

    adc_gpio_init(TOUCH_XP);
    adc_select_input(0);

    sleep_us(100);
    return adc_read();
}

static int read_avg_x(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += touch_read_x();
        sleep_us(80);
    }
    return sum / 10;
}

static int read_avg_y(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += touch_read_y();
        sleep_us(80);
    }
    return sum / 10;
}

void touch_get_raw(int *x, int *y) {
    *x = read_avg_x();
    *y = read_avg_y();
}

int touch_is_pressed(int raw_x, int raw_y) {
    (void)raw_x;
    return raw_y > 220;
}

void touch_get_screen(int *x, int *y, int *pressed) {
    int raw_x;
    int raw_y;

    touch_get_raw(&raw_x, &raw_y);
    *pressed = touch_is_pressed(raw_x, raw_y);

    if (!(*pressed)) {
        *x = -1;
        *y = -1;
        return;
    }

    int sx = map_int(raw_x, RAW_X_MIN, RAW_X_MAX, 0, SCREEN_W - 1);
    int sy = map_int(raw_y, RAW_Y_MIN, RAW_Y_MAX, 0, SCREEN_H - 1);

    sx = clamp_int(sx, 0, SCREEN_W - 1);
    sy = clamp_int(sy, 0, SCREEN_H - 1);

    *x = sx;
    *y = sy;

}

static void handle_touch_zone(int x, int y) {
    printf("ZONE TEST x=%d y=%d\n", x, y);

    // TOP RIGHT = toggle focus mode
    if (x > 135 && y < 60) {
        deskbot_toggle_pomodoro();

        printf("Action: toggle pomodoro\n");

        cloud_send_command("/api/deskbot/toggle");
        return;
    }

    // MIDDLE LOWER = next task
    if (y >= 205 && y <= 264) {
        printf("Action: next task\n");

        cloud_send_command("/api/deskbot/next");
        return;
    }

    // BOTTOM = reset timer
    if (y > 265) {
        deskbot_reset_pomodoro();

        printf("Action: reset pomodoro\n");

        cloud_send_command("/api/deskbot/reset");
        return;
    }

    printf("Action: none\n");
}

void touch_task(void *p) {
    (void)p;

    int was_pressed = 0;
    absolute_time_t last_touch_time = get_absolute_time();

    while (1) {
        int x;
        int y;
        int pressed;

        touch_get_screen(&x, &y, &pressed);

        if (pressed && !was_pressed) {
            int64_t since_ms =
                absolute_time_diff_us(last_touch_time, get_absolute_time()) / 1000;

            if (since_ms > 700) {
                printf("Touch: %d,%d\n", x, y);
                handle_touch_zone(x, y);
                last_touch_time = get_absolute_time();
            }

            was_pressed = 1;
        }

        if (!pressed) {
            was_pressed = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(80));
    }
}