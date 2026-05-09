#include "pomodoro.h"
#include "deskbot_state.h"

#include "FreeRTOS.h"
#include "task.h"

void pomodoro_task(void *params) {
    (void)params;

    bot_state_t *state = deskbot_get_state();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (state->pomodoro_running && state->seconds_left > 0) {
            state->seconds_left--;
            state->mood = BOT_FOCUS;
        }

        if (state->pomodoro_running && state->seconds_left <= 0) {
            state->pomodoro_running = 0;
            state->mood = BOT_BREAK;
        }
    }
}
