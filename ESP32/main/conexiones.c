#include "packeting.c"
#include <stdlib.h>
#include <string.h>
#include "lwip/sys.h"
#include "sdkconfig.h"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_wifi.h"

#define HOST_IP_ADDR "192.168.4.1"
#define PORT_TCP 8000
#define PORT_UDP 8001

const char *TAG = "conexiones";

int create_TCP_socket()
{
  struct sockaddr_in dest_addr;
  inet_pton(AF_INET, HOST_IP_ADDR, &dest_addr.sin_addr);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(PORT_TCP);

  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_IP;

  int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
  if (sock < 0)
  {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    return -1;
  }

  ESP_LOGI(TAG, "Socket created, connecting to %s:%d", HOST_IP_ADDR, PORT_TCP);
  int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  if (err != 0)
  {
    ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
    return -1;
  }
  ESP_LOGI(TAG, "Successfully connected");

  return sock;
}

int create_UDP_socket()
{
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(PORT_UDP);

  int addr_family = AF_INET;
  int ip_protocol = IPPROTO_IP;

  int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
  if (sock < 0)
  {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    return -1;
  }

  // Set timeout
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

  ESP_LOGI(TAG, "Socket created, connecting to %s:%d", HOST_IP_ADDR, PORT_UDP);
  int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  if (err != 0)
  {
    ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
    return -1;
  }
  ESP_LOGI(TAG, "Successfully connected");

  return sock;
}

void close_socket(int sock)
{
  ESP_LOGI(TAG, "Shutting down socket");
  shutdown(sock, 0);
  close(sock);
}

int TCP_send_frag(int sock, char protocolo)
{
  // Parte el mensaje (payload) en trozos de 1000 btyes y los manda por separado, esperando un OK con cada trozo
  char status = '1';
  char *payload = mensaje(protocolo, status);
  int payloadLen = messageLength(protocolo);
  int PACK_LEN = 1024;
  char rx_buffer[1024];

  for (int i = 0; i < payloadLen; i += PACK_LEN)
  {

    // Generamos el siguiente trozo
    int size = fmin(PACK_LEN, payloadLen - i);
    char *pack = malloc(size);
    memcpy(pack, &(payload[i]), size);

    // Enviamos el trozo
    int err = send(sock, pack, size, 0);
    free(pack);
    if (err < 0)
    {
      ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }

    // wait for confirmation
    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    // Error occurred during receiving
    if (len < 0)
    {
      // En caso de error abortamos
      ESP_LOGE(TAG, "recv failed: errno %d", errno);
      free(payload);
      return -1;
    }
    else
    {
      rx_buffer[len] = 0;
      char OK_r = rx_buffer[0];
      if (!OK_r)
      {
        ESP_LOGE(TAG, "Server error in fragmented send.");
        free(payload);
        return -1;
      }
    }
  }
  // el último mensaje es solo un \0 para avisarle al server que terminamos
  int err = send(sock, "\0", 1, 0);

  free(payload);

  return err;
}

int UDP_send_frag(int sock, char protocolo)
{
  char status = '0';
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(PORT_UDP);

  // Parte el mensaje (payload) en trozos de 1000 btyes y los manda por separado, esperando un OK con cada trozo
  char *payload = mensaje(protocolo, status);
  int payloadLen = messageLength(protocolo);
  int PACK_LEN = 1024;
  char rx_buffer[1024];

  for (int i = 0; i < payloadLen; i += PACK_LEN)
  {

    // Generamos el siguiente trozo
    int size = fmin(PACK_LEN, payloadLen - i);
    char *pack = malloc(size);
    memcpy(pack, &(payload[i]), size);

    // Enviamos el trozo
    int err = sendto(sock, pack, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    free(pack);
    if (err < 0)
    {
      ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
      break;
    }

    // wait for confirmation
    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    // Error occurred during receiving
    if (len < 0)
    {
      // En caso de error abortamos
      ESP_LOGE(TAG, "recv failed: errno %d", errno);
      free(payload);
      return -1;
    }
    else
    {
      rx_buffer[len] = 0;
      char OK_r = rx_buffer[0];
      if (!OK_r)
      {
        ESP_LOGE(TAG, "Server error in fragmented send.");
        free(payload);
        return -1;
      }
    }
  }
  // el último mensaje es solo un \0 para avisarle al server que terminamos
  int err = sendto(sock, "\0", 1, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

  free(payload);

  return err;
}
