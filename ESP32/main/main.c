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

#include "esp_sntp.h"
#include "esp_sleep.h"

extern int create_TCP_socket();
extern int create_UDP_socket();
extern void close_socket(int sock);
extern int TCP_send_frag(int sock, char protocolo);
extern int UDP_send_frag(int sock, char protocolo);

char *fetch_config(int socket);

char transportLayer = '1'; // 1 = TCP, 0 = UDP
char protocol = '0';

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

  esp_sleep_enable_timer_wakeup(60 * 1000 * 1000);

  int sock_UDP = create_UDP_socket();
  int sock_TCP = create_TCP_socket();
  while (1)
  {
    char *config = fetch_config(sock_TCP);
    while (config == NULL)
    {
      ESP_LOGE("main", "Error occurred during fetching config");
      close_socket(sock_TCP);
      int sock_TCP = create_TCP_socket();
      close_socket(sock_UDP);
      int sock_UDP = create_UDP_socket();
      config = fetch_config(sock_TCP);
      sleep(15);
    }
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
          close_socket(sock_TCP);
          int sock_TCP = create_TCP_socket();
          close_socket(sock_UDP);
          int sock_UDP = create_UDP_socket();
          break;
        }

        ESP_LOGI("TCP", "Message sent");
        sleep(60);
        // esp_deep_sleep_start();
      }
      else
      {
        int err = UDP_send_frag(sock_UDP, protocol);

        if (err < 0)
        {
          ESP_LOGE("main", "Error occurred during sending: errno %d", err);
          close_socket(sock_UDP);
          int sock_UDP = create_UDP_socket();
          close_socket(sock_TCP);
          int sock_TCP = create_TCP_socket();
          break;
        }

        ESP_LOGI("UDP", "Message sent");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }
    }
  }
}

// Set the system time from a Unix timestamp
void set_system_time(time_t timestamp)
{
  struct timeval tv = {.tv_sec = timestamp};
  settimeofday(&tv, NULL);
}

char *fetch_config(int sock)
{
  char *config = malloc(1024 * sizeof(char));
  int len = recv(sock, config, sizeof(config) - 1, 0);
  if (len < 0)
  {
    return NULL;
  }
  ESP_LOGI("fetch_config", "config: %s", config);
  config[len] = '\0';

  long long unix_timestamp;
  int len2 = recv(sock, &unix_timestamp, sizeof(unix_timestamp), 0);
  if (len2 < 0)
  {
    return NULL;
  }
  ESP_LOGI("fetch_config", "rb Unix timestamp: %lld\n", unix_timestamp);
  set_system_time(unix_timestamp);
  return (char *)config;
}
