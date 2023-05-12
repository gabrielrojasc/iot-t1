/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "esp_log.h"

#include <stdio.h>
#include <unistd.h>

#include "driver/gpio.h"

#define BUTTON_PIN 21

extern int create_TCP_socket();
extern int create_UDP_socket();
extern void close_socket(int sock);
extern int TCP_send_frag(int sock, char status, char protocolo);
extern int UDP_send_frag(int sock, char status, char protocolo);

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    int buttonStatePrev = 1;
    int buttonStateNow = 1;
    int justChanged = 0;

    char transportLayer = '1';
    int sock_TCP = create_TCP_socket();
    int sock_UDP = create_UDP_socket();
    char protocol = '0';
    int protocolInt = 0;
    while (1)
    {
        protocolInt++;
        protocolInt %= 5;
        protocol = protocolInt + '0';
        buttonStatePrev = buttonStateNow;
        buttonStateNow = gpio_get_level(BUTTON_PIN);
        if (!buttonStateNow && buttonStatePrev)
        {
            justChanged = 1;
            if (transportLayer == '1')
            {
                transportLayer = '0';
                ESP_LOGI("main", "Transport layer: UDP");
            }
            else
            {
                transportLayer = '1';
                ESP_LOGI("main", "Transport layer: TCP");
            }
        }
        if (transportLayer == '1')
        {
            int err = TCP_send_frag(sock_TCP, transportLayer, protocol);

            if (err < 0)
            {
                ESP_LOGE("main", "Error occurred during sending: errno %d", err);
                break;
            }

            ESP_LOGI("TCP", "Message sent");
        }
        else
        {
            int err = UDP_send_frag(sock_UDP, transportLayer, protocol);

            if (err < 0)
            {
                ESP_LOGE("main", "Error occurred during sending: errno %d", err);
                break;
            }

            ESP_LOGI("UDP", "Message sent");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
