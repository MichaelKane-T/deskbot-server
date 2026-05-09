#pragma once

typedef enum {
    BOT_IDLE,
    BOT_FOCUS,
    BOT_BREAK
} bot_mood_t;

typedef struct {
    bot_mood_t mood;
    int seconds_left;
    int pomodoro_running;
    char task[64];
    char project[32];
    char time_text[16];
} bot_state_t;

void deskbot_state_init(void);
bot_state_t* deskbot_get_state(void);

void deskbot_toggle_pomodoro(void);
void deskbot_reset_pomodoro(void);
