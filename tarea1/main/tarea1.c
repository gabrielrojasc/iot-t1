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

extern void tcp_client(void);
extern void udp_client(void *pvParameters);

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

    while (false)
    {
        // buttonStatePrev = buttonStateNow;
        // buttonStateNow = gpio_get_level(BUTTON_PIN);
        // ESP_LOGI("main", "Button state: %d", buttonStateNow);
        // tcp_client();
        // ESP_LOGI("main", "Reconnecting");
    }
    xTaskCreate(udp_client, "udp_client", 4096, NULL, 5, NULL);
}
