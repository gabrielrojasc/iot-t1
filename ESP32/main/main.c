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
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include "driver/gpio.h"

#define BUTTON_PIN 21

extern int create_TCP_socket();
extern int create_UDP_socket();
extern void close_socket(int sock);
extern int TCP_send_frag(int sock, char protocolo);
extern int UDP_send_frag(int sock, char protocolo);

char transportLayer = '1'; // 1 = TCP, 0 = UDP
char protocol = '0';

char *fetch_config(int socket);

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

  int sock_UDP = create_UDP_socket();
  int sock_TCP = create_TCP_socket();
  while (1)
  {
    char *config = fetch_config(sock_TCP);
    protocol = config[0];
    transportLayer = config[1];
    free(config);

    while (1)
    {

      if (transportLayer == '1')
      {
        int err = TCP_send_frag(sock_TCP, protocol);

        if (err < 0)
        {
          ESP_LOGE("main", "Error occurred during sending: errno %d", err);
          // close_socket(sock_TCP);
          // int sock_TCP = create_TCP_socket();
          break;
        }

        ESP_LOGI("TCP", "Message sent");
        sleep(60);
      }
      else
      {
        int err = UDP_send_frag(sock_UDP, protocol);

        if (err < 0)
        {
          ESP_LOGE("main", "Error occurred during sending: errno %d", err);
          // close_socket(sock_UDP);
          // int sock_UDP = create_UDP_socket();
          break;
        }

        ESP_LOGI("UDP", "Message sent");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }
    }
  }
}

char *fetch_config(int sock)
{
  char *config = malloc(1024 * sizeof(char));
  int len = recv(sock, config, sizeof(config) - 1, 0);
  return config;
}
