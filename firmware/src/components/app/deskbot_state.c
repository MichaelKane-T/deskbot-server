#include "deskbot_state.h"
#include <string.h>

static bot_state_t state;

void deskbot_state_init(void) {
    state.mood = BOT_IDLE;
    state.seconds_left = 25 * 60;
    state.pomodoro_running = 0;

    strcpy(state.task, "Work on RoverPi");
    strcpy(state.project, "RoverPi");
}

bot_state_t* deskbot_get_state(void) {
    return &state;
}

void deskbot_toggle_pomodoro(void) {
    state.pomodoro_running = !state.pomodoro_running;
    state.mood = state.pomodoro_running ? BOT_FOCUS : BOT_IDLE;
}

void deskbot_reset_pomodoro(void) {
    state.seconds_left = 25 * 60;
    state.pomodoro_running = 0;
    state.mood = BOT_IDLE;
}
