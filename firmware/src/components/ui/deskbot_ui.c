#include "deskbot_ui.h"
#include "deskbot_state.h"
#include "ili9341.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CLOCK_Y     16
#define STATUS_Y    16

#define FACE_Y0      75
#define FACE_H       120

#define TASK_Y      222
#define PROJECT_Y   244
#define POMO_Y      268
#define BAR_Y       298

#define COLOR_BLACK       ILI9341_BLACK
#define COLOR_WHITE       ILI9341_WHITE
#define COLOR_EYES        ILI9341_CYAN
#define COLOR_FOCUS       0x07E0
#define COLOR_FOCUS_DIM   0x03A0
#define COLOR_POMO        0xFD20
#define COLOR_BAR_BG      0x39E7

static void draw_rect(int x, int y, int w, int h, uint16_t color) {
    ili9341_fill_rect(x, y, w, h, color);
}

/* Tiny 5x7 font */
static const uint8_t *font_for_char(char c) {
    static const uint8_t SPACE[5]={0,0,0,0,0};
    static const uint8_t A[5]={0x7E,0x11,0x11,0x11,0x7E};
    static const uint8_t B[5]={0x7F,0x49,0x49,0x49,0x36};
    static const uint8_t C[5]={0x3E,0x41,0x41,0x41,0x22};
    static const uint8_t D[5]={0x7F,0x41,0x41,0x22,0x1C};
    static const uint8_t E[5]={0x7F,0x49,0x49,0x49,0x41};
    static const uint8_t F[5]={0x7F,0x09,0x09,0x09,0x01};
    static const uint8_t G[5]={0x3E,0x41,0x49,0x49,0x7A};
    static const uint8_t H[5]={0x7F,0x08,0x08,0x08,0x7F};
    static const uint8_t I[5]={0x00,0x41,0x7F,0x41,0x00};
    static const uint8_t J[5]={0x20,0x40,0x41,0x3F,0x01};
    static const uint8_t K[5]={0x7F,0x08,0x14,0x22,0x41};
    static const uint8_t L[5]={0x7F,0x40,0x40,0x40,0x40};
    static const uint8_t M[5]={0x7F,0x02,0x0C,0x02,0x7F};
    static const uint8_t N[5]={0x7F,0x04,0x08,0x10,0x7F};
    static const uint8_t O[5]={0x3E,0x41,0x41,0x41,0x3E};
    static const uint8_t P[5]={0x7F,0x09,0x09,0x09,0x06};
    static const uint8_t Q[5]={0x3E,0x41,0x51,0x21,0x5E};
    static const uint8_t R[5]={0x7F,0x09,0x19,0x29,0x46};
    static const uint8_t S[5]={0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[5]={0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t U[5]={0x3F,0x40,0x40,0x40,0x3F};
    static const uint8_t V[5]={0x1F,0x20,0x40,0x20,0x1F};
    static const uint8_t W[5]={0x7F,0x20,0x18,0x20,0x7F};
    static const uint8_t X[5]={0x63,0x14,0x08,0x14,0x63};
    static const uint8_t Y[5]={0x03,0x04,0x78,0x04,0x03};
    static const uint8_t Z[5]={0x61,0x51,0x49,0x45,0x43};

    static const uint8_t N0[5]={0x3E,0x51,0x49,0x45,0x3E};
    static const uint8_t N1[5]={0x00,0x42,0x7F,0x40,0x00};
    static const uint8_t N2[5]={0x42,0x61,0x51,0x49,0x46};
    static const uint8_t N3[5]={0x21,0x41,0x45,0x4B,0x31};
    static const uint8_t N4[5]={0x18,0x14,0x12,0x7F,0x10};
    static const uint8_t N5[5]={0x27,0x45,0x45,0x45,0x39};
    static const uint8_t N6[5]={0x3C,0x4A,0x49,0x49,0x30};
    static const uint8_t N7[5]={0x01,0x71,0x09,0x05,0x03};
    static const uint8_t N8[5]={0x36,0x49,0x49,0x49,0x36};
    static const uint8_t N9[5]={0x06,0x49,0x49,0x29,0x1E};

    static const uint8_t COLON[5]={0x00,0x36,0x36,0x00,0x00};
    static const uint8_t DASH[5]={0x08,0x08,0x08,0x08,0x08};

    if (c >= 'a' && c <= 'z') c -= 32;

    switch(c) {
        case 'A': return A; case 'B': return B; case 'C': return C; case 'D': return D;
        case 'E': return E; case 'F': return F; case 'G': return G; case 'H': return H;
        case 'I': return I; case 'J': return J; case 'K': return K; case 'L': return L;
        case 'M': return M; case 'N': return N; case 'O': return O; case 'P': return P;
        case 'Q': return Q; case 'R': return R; case 'S': return S; case 'T': return T;
        case 'U': return U; case 'V': return V; case 'W': return W; case 'X': return X;
        case 'Y': return Y; case 'Z': return Z;

        case '0': return N0; case '1': return N1; case '2': return N2; case '3': return N3;
        case '4': return N4; case '5': return N5; case '6': return N6; case '7': return N7;
        case '8': return N8; case '9': return N9;

        case ':': return COLON;
        case '-': return DASH;
        default: return SPACE;
    }
}

static void draw_char(int x, int y, char c, uint16_t color, int scale) {
    const uint8_t *bitmap = font_for_char(c);

    for (int col = 0; col < 5; col++) {
        uint8_t line = bitmap[col];

        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                draw_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

static void draw_string(int x, int y, const char *s, uint16_t color, int scale) {
    while (*s) {
        draw_char(x, y, *s, color, scale);
        x += 6 * scale;
        s++;
    }
}

static uint16_t focus_color(int step) {
    return (step % 2) ? COLOR_FOCUS : COLOR_FOCUS_DIM;
}

static const char *mood_text(bot_mood_t mood) {
    switch (mood) {
        case BOT_FOCUS: return "FOCUS";
        case BOT_BREAK: return "BREAK";
        default: return "IDLE";
    }
}

static uint16_t mood_color(bot_mood_t mood, int step) {
    if (mood == BOT_FOCUS) return focus_color(step);
    if (mood == BOT_BREAK) return COLOR_POMO;
    return COLOR_WHITE;
}

static void draw_top_bar(bot_state_t *state, int pulse_step) {
    draw_rect(0, 0, TFT_WIDTH, 42, COLOR_BLACK);

    draw_string(8, CLOCK_Y, state->time_text[0] ? state->time_text : "--:--", COLOR_WHITE, 2);

    const char *txt = mood_text(state->mood);
    int scale = 2;
    int len = 0;
    while (txt[len]) len++;

    int text_w = len * 6 * scale;
    int x = TFT_WIDTH - text_w - 8;

    draw_string(x, STATUS_Y, txt, mood_color(state->mood, pulse_step), scale);
}

static int idle_eye_offset(int step) {
    switch (step % 5) {
        case 1: return 3;
        case 3: return -3;
        default: return 0;
    }
}

static void draw_eyes(int closed, int eye_offset_x) {
    int center_x = (TFT_WIDTH / 2) + eye_offset_x;

    if (closed) {
        int eye_w = 30;
        int eye_h = 5;
        int eye_y = 108;
        int eye_gap = 52;

        draw_rect(center_x - eye_gap, eye_y, eye_w, eye_h, COLOR_EYES);
        draw_rect(center_x + eye_gap - eye_w, eye_y, eye_w, eye_h, COLOR_EYES);
    } else {
        int eye_w = 24;
        int eye_h = 24;
        int eye_y = 96;
        int eye_gap = 50;

        draw_rect(center_x - eye_gap, eye_y, eye_w, eye_h, COLOR_EYES);
        draw_rect(center_x + eye_gap - eye_w, eye_y, eye_w, eye_h, COLOR_EYES);
    }
}

static void draw_smile(void) {
    int cx = TFT_WIDTH / 2;
    int y = 165;

    draw_rect(cx - 36, y + 0, 8, 4, COLOR_WHITE);
    draw_rect(cx - 28, y + 5, 8, 4, COLOR_WHITE);
    draw_rect(cx - 20, y + 9, 8, 4, COLOR_WHITE);
    draw_rect(cx - 12, y + 12, 24, 4, COLOR_WHITE);
    draw_rect(cx + 12, y + 9, 8, 4, COLOR_WHITE);
    draw_rect(cx + 20, y + 5, 8, 4, COLOR_WHITE);
    draw_rect(cx + 28, y + 0, 8, 4, COLOR_WHITE);
}

static void draw_face(bot_state_t *state, int closed, int eye_step) {
    int eye_offset = 0;

    if (state->mood == BOT_IDLE) {
        eye_offset = idle_eye_offset(eye_step);
    }

    draw_rect(0, FACE_Y0, TFT_WIDTH, FACE_H, COLOR_BLACK);
    draw_eyes(closed, eye_offset);
    draw_smile();
}

static void draw_pomodoro_bar(bot_state_t *state) {
    int total = 25 * 60;
    int left = state->seconds_left;

    if (left < 0) left = 0;
    if (left > total) left = total;

    int x = 8;
    int w = TFT_WIDTH - 16;
    int h = 8;
    int filled = (w * left) / total;

    draw_rect(x, BAR_Y, w, h, COLOR_BAR_BG);
    draw_rect(x + 1, BAR_Y + 1, w - 2, h - 2, COLOR_BLACK);

    if (filled > 2) {
        draw_rect(x + 1, BAR_Y + 1, filled - 2, h - 2, COLOR_POMO);
    }
}

static void draw_bottom_text(bot_state_t *state) {
    char line[64];

    draw_rect(0, 210, TFT_WIDTH, 110, COLOR_BLACK);

    snprintf(line, sizeof(line), "TASK: %s", state->task);
    draw_string(8, TASK_Y, line, COLOR_WHITE, 1);

    snprintf(line, sizeof(line), "PROJECT: %s", state->project);
    draw_string(8, PROJECT_Y, line, COLOR_WHITE, 1);

    int minutes = state->seconds_left / 60;
    int seconds = state->seconds_left % 60;

    snprintf(line, sizeof(line), "POMODORO: %02d:%02d LEFT", minutes, seconds);
    draw_string(8, POMO_Y, line, COLOR_POMO, 1);

    draw_pomodoro_bar(state);
}

static void draw_frame(bot_state_t *state, int closed, int pulse_step, int eye_step) {
    draw_top_bar(state, pulse_step);
    draw_face(state, closed, eye_step);
    draw_bottom_text(state);
}

void ui_task(void *params) {
    (void)params;

    bot_state_t *state = deskbot_get_state();

    ili9341_fill_screen(COLOR_BLACK);

    int pulse_step = 0;
    int eye_step = 0;

    while (1) {
    draw_top_bar(state, pulse_step);
    draw_bottom_text(state);
    draw_face(state, 0, eye_step);

    pulse_step++;
    eye_step++;

    vTaskDelay(pdMS_TO_TICKS(1800));

    draw_face(state, 1, eye_step);
    vTaskDelay(pdMS_TO_TICKS(120));
}
}
