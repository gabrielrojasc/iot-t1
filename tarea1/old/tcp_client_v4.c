#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_wifi.h"

#define PORT 8000
#define HOST_IP_ADDR "192.168.4.1"

static const char *TAG = "example";
static const char *payload = "Message from ESP32 ";

extern int TCP_send_frag(int sock, char frag1, char frag2);

int create_and_connect_TCP_socket()
{
    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, HOST_IP_ADDR, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", HOST_IP_ADDR, PORT);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return -1;
    }
    ESP_LOGI(TAG, "Successfully connected");

    return sock;
}

void close_TCP_socket(int sock)
{
    ESP_LOGI(TAG, "Shutting down socket");
    shutdown(sock, 0);
    close(sock);
}

// void tcp_client(void)
// {
//     char rx_buffer[128];
//     char host_ip[] = HOST_IP_ADDR;
//     int addr_family = 0;
//     int ip_protocol = 0;

//     while (1)
//     {
//         struct sockaddr_in dest_addr;
//         inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;

//         int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
//         if (sock < 0)
//         {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

//         int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//         if (err != 0)
//         {
//             ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Successfully connected");

//         while (1)
//         {
//             // int err = send(sock, payload, strlen(payload), 0);
//             int err = TCP_send_frag(sock, '0', '0');
//             if (err < 0)
//             {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break;
//             }
//         }

//         if (sock != -1)
//         {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }
//     }
// }
