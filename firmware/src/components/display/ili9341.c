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
    data[0] = (uint8_t)(x0 >> 8); data[1] = (uint8_t)x0;
    data[2] = (uint8_t)(x1 >> 8); data[3] = (uint8_t)x1;
    write_data(data, 4);

    write_cmd(0x2B);
    data[0] = (uint8_t)(y0 >> 8); data[1] = (uint8_t)y0;
    data[2] = (uint8_t)(y1 >> 8); data[3] = (uint8_t)y1;
    write_data(data, 4);

    write_cmd(0x2C);
}

void ili9341_init(void) {
    spi_init(TFT_SPI, 40000000); // safer for breadboard wiring

    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_put(PIN_DC, 1);

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);

    gpio_init(PIN_LITE);
    gpio_set_dir(PIN_LITE, GPIO_OUT);
    gpio_put(PIN_LITE, 1);

    // Hardware reset
    gpio_put(PIN_RST, 1);
    sleep_ms(50);
    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);
    sleep_ms(150);

    // Software reset
    write_cmd(0x01);
    sleep_ms(150);

    // Display off
    write_cmd(0x28);

    // Memory access control
    // 0x48 usually fixes rotation/color order better on many ILI9341 boards
    write_cmd(0x36);
    write_data_u8(0x48);

    // Pixel format: 16-bit RGB565
    write_cmd(0x3A);
    write_data_u8(0x55);

    // Sleep out
    write_cmd(0x11);
    sleep_ms(150);

    // Display on
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

    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);
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

static const uint8_t FONT_SPACE[5] = {0,0,0,0,0};

static const uint8_t* font_for_char(char c) {
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
    static const uint8_t SLASH[5]={0x20,0x10,0x08,0x04,0x02};
    static const uint8_t DOT[5]={0x00,0x60,0x60,0x00,0x00};

    if (c >= 'a' && c <= 'z') c -= 32;

    switch (c) {
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
        case '/': return SLASH;
        case '.': return DOT;
        case ' ': return FONT_SPACE;
        default: return FONT_SPACE;
    }
}

void ili9341_draw_char(int x, int y, char c, uint16_t color, uint16_t bg, int scale) {
    const uint8_t *bitmap = font_for_char(c);

    for (int col = 0; col < 5; col++) {
        uint8_t line = bitmap[col];

        for (int row = 0; row < 7; row++) {
            uint16_t pixel_color = (line & (1 << row)) ? color : bg;
            ili9341_fill_rect(
                x + col * scale,
                y + row * scale,
                scale,
                scale,
                pixel_color
            );
        }
    }

    // spacing column
    ili9341_fill_rect(x + 5 * scale, y, scale, 7 * scale, bg);
}

void ili9341_draw_rgb565_buffer(int x, int y, int w, int h, const uint16_t *buffer) {
    if (x < 0 || y < 0 || x + w > TFT_WIDTH || y + h > TFT_HEIGHT) return;

    set_addr_window(x, y, x + w - 1, y + h - 1);

    cs_select();
    dc_data();

    for (int i = 0; i < w * h; i++) {
        uint8_t data[2];
        data[0] = (uint8_t)(buffer[i] >> 8);
        data[1] = (uint8_t)(buffer[i] & 0xFF);
        spi_write_blocking(TFT_SPI, data, 2);
    }

    cs_deselect();
}

void ili9341_draw_string(int x, int y, const char *str, uint16_t color, uint16_t bg, int scale) {
    while (*str) {
        ili9341_draw_char(x, y, *str, color, bg, scale);
        x += 6 * scale;
        str++;
    }
}
