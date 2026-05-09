#include <stdio.h>

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include "ili9341.h"
#include "deskbot_state.h"
#include "pomodoro.h"
#include "deskbot_ui.h"
#include "touch.h"
#include "cloud_sync.h"

void vApplicationMallocFailedHook(void) {
    printf("Malloc failed!\n");
    taskDISABLE_INTERRUPTS();
    while (1) {}
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    printf("Stack overflow: %s\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    while (1) {}
}

static void check_task(BaseType_t ok, const char *name) {
    if (ok != pdPASS) {
        printf("Task create failed: %s\n", name);
    } else {
        printf("Task created: %s\n", name);
    }
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("DeskBot booting...\n");

    ili9341_init();
    printf("Display init done\n");

    deskbot_state_init();
    printf("State init done\n");

    touch_init();
    printf("Touch init done\n");

    check_task(xTaskCreate(ui_task, "UI", 1536, NULL, 1, NULL), "UI");
    check_task(xTaskCreate(touch_task, "TOUCH", 768, NULL, 1, NULL), "TOUCH");
    check_task(xTaskCreate(pomodoro_task, "POMO", 512, NULL, 1, NULL), "POMO");
    xTaskCreate(cloud_sync_task, "CLOUD", 4096, NULL, 1, NULL);

    printf("Starting scheduler...\n");

    vTaskStartScheduler();

    printf("Scheduler failed to start!\n");

    while (1) {
        sleep_ms(1000);
    }
}