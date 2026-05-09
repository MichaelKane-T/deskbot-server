#include "cloud_sync.h"
#include "cloud_config.h"
#include "deskbot_state.h"

#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

#include "cloud_sync.h"

static char response_buf[1024];

static void extract_string(const char *json, const char *key, char *out, int max_len) {
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);

    char *start = strstr(json, pattern);
    if (!start) return;

    start += strlen(pattern);
    char *end = strchr(start, '"');
    if (!end) return;

    int len = end - start;
    if (len >= max_len) len = max_len - 1;

    memcpy(out, start, len);
    out[len] = '\0';
}

static void apply_cloud_json(const char *json) {
    bot_state_t *state = deskbot_get_state();

    extract_string(json, "time", state->time_text, sizeof(state->time_text));
    extract_string(json, "active_task", state->task, sizeof(state->task));
    extract_string(json, "mode", state->project, sizeof(state->project));

    if (strstr(json, "\"running\":true")) {
        state->pomodoro_running = 1;
        state->mood = BOT_FOCUS;
    } else if (strstr(json, "\"running\":false")) {
        state->pomodoro_running = 0;
        state->mood = BOT_IDLE;
    }

    printf("Cloud update: time=%s task=%s project=%s running=%d\n",
           state->time_text,
           state->task,
           state->project,
           state->pomodoro_running);
}

static int response_len = 0;

static err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                           struct pbuf *p, err_t err) {
    (void)arg;
    (void)err;

    if (!p) {
        response_buf[response_len] = '\0';

        printf("Full server response:\n%s\n", response_buf);

        char *json = strstr(response_buf, "\r\n\r\n");
        if (json) {
            json += 4;

            printf("Parsed JSON: %s\n", json);
            apply_cloud_json(json);
        }

        response_len = 0;
        tcp_close(tpcb);
        return ERR_OK;
    }

    int copy_len = p->tot_len;

    if (response_len + copy_len >= (int)sizeof(response_buf)) {
        copy_len = sizeof(response_buf) - response_len - 1;
    }

    if (copy_len > 0) {
        pbuf_copy_partial(p, response_buf + response_len, copy_len, 0);
        response_len += copy_len;
    }

    pbuf_free(p);
    return ERR_OK;
}

static err_t connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    (void)arg;

    if (err != ERR_OK) {
        printf("TCP connect callback error: %d\n", err);
        tcp_close(tpcb);
        return err;
    }

    response_len = 0;

    char request[256];
    snprintf(request, sizeof(request),
         "GET /api/deskbot/state HTTP/1.1\r\n"
         "Host: %s\r\n"
         "X-Pico-Key: deskbot-pico-2026\r\n"
         "Connection: close\r\n"
         "\r\n",
         DESKBOT_SERVER_IP);

    tcp_recv(tpcb, recv_callback);
    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    return ERR_OK;
}

void cloud_sync_task(void *params) {
    (void)params;

    printf("Cloud sync starting...\n");

    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        vTaskDelete(NULL);
    }

    cyw43_arch_enable_sta_mode();

    while (1) {
        printf("Connecting to WiFi...\n");

        if (cyw43_arch_wifi_connect_timeout_ms(
                WIFI_SSID,
                WIFI_PASS,
                CYW43_AUTH_WPA2_AES_PSK,
                30000)) {
            printf("WiFi connect failed, retrying...\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        printf("WiFi connected\n");
        break;
    }

    while (1) {
        struct tcp_pcb *pcb = tcp_new();

        if (!pcb) {
            printf("TCP alloc failed\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        ip_addr_t server_ip;
        if (!ip4addr_aton(DESKBOT_SERVER_IP, &server_ip)) {
            printf("Bad server IP\n");
            tcp_abort(pcb);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        err_t err = tcp_connect(
            pcb,
            &server_ip,
            DESKBOT_SERVER_PORT,
            connect_callback
        );

        if (err != ERR_OK) {
            printf("TCP connect failed: %d\n", err);
            tcp_abort(pcb);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static const char *pending_command = NULL;

static err_t command_connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    const char *path = (const char *)arg;

    if (err != ERR_OK) {
        printf("Command connect failed: %d\n", err);
        tcp_close(tpcb);
        return err;
    }

    char request[256];

    snprintf(request, sizeof(request),
             "POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Length: 0\r\n"
             "Connection: close\r\n"
             "\r\n",
             path,
             DESKBOT_SERVER_IP);

    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    printf("Sent command: %s\n", path);

    tcp_close(tpcb);
    return ERR_OK;
}

void cloud_send_command(const char *path) {
    if (!path) return;

    struct tcp_pcb *pcb = tcp_new();

    if (!pcb) {
        printf("Command TCP alloc failed\n");
        return;
    }

    ip_addr_t server_ip;
    if (!ip4addr_aton(DESKBOT_SERVER_IP, &server_ip)) {
        printf("Bad server IP for command\n");
        tcp_abort(pcb);
        return;
    }

    tcp_arg(pcb, (void *)path);

    err_t err = tcp_connect(
        pcb,
        &server_ip,
        DESKBOT_SERVER_PORT,
        command_connect_callback
    );

    if (err != ERR_OK) {
        printf("Command tcp_connect failed: %d\n", err);
        tcp_abort(pcb);
    }

    pending_command = path;
}