/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "./tcp_client_v4.h"


// #if defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
// #include "addr_from_stdin.h"
// #endif

// #if defined(CONFIG_EXAMPLE_IPV4)
// #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
// #elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
// #define HOST_IP_ADDR ""
// #endif

#define HOST_IP_ADDR "192.168.2.100"
#define PORT 3333


static const char *TAG = "tcp_client";
// static const char *payload = "Message from ESP32 ";




/*         int len = sprintf(message_data, "{\
\"uuid\": \"%02x%02x%02x%02x%02x%02x\",\
\"type\": \"CONNECTION_DATA\"\
}", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            ESP_LOGI(WS_TAG, "Sending %s", message_data);
            esp_websocket_client_send_text(client, message_data, len, portMAX_DELAY);



                int len = sprintf(message_data, "{\
\"uuid\": \"%02x%02x%02x%02x%02x%02x\",\
\"type\": \"SEND_TOKEN\",\
\"payload\": {\
        \"token\": \"%s\"\
    }\
}", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], char_key );
                ESP_LOGI(WS_TAG, "Sending %s", message_data);
                esp_websocket_client_send_text(client, message_data, len, portMAX_DELAY);

 */
// static const char *WS_TAG = "web-socket";
// // static const char *WS_ADRRESS = "ws://192.168.0.106:3100";
// // static const char *WS_ADRRESS = "ws://82.196.12.78:3100";
// static const char *WS_ADRRESS = "ws://192.168.0.114:3100";



static QueueHandle_t _tcp_sending_queue;
static QueueHandle_t _tcp_receiving_queue;



// static void udp_client_send_task(void *pvParameters)
// {
//     ESP_LOGI(TAG, "udp_client_send_task");
    
//     int addr_family = 0;
//     int ip_protocol = 0;

//     while (1) {
//         struct sockaddr_in dest_addr;
//         dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;

//         int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
//         if (sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

//         UDPItem_t new_msg;

//         while (1) {
//             // counter++;

//             if (xQueueReceive(tcp_sending_queue, (void *)&new_msg, 1) == pdTRUE)
//             {
//                 counter++;
//                 // printf("new_msg.id: %d %d\n", new_msg.id[1], new_msg.id[0]);
//                 // printf(" new_msg.data_length: %d\n", new_msg.data_length);
//                 // printf("new_msg.data: %s", new_msg.data);
//                 char _payload[200] = { 0 };
//                 // _payload[0] = new_msg.id[0];
//                 // _payload[1] = new_msg.id[1];
//                 // memcpy(&_payload[2], &new_msg.data, new_msg.data_length);
//                 // int err = sendto(sock, _payload, new_msg.data_length + 2 /* 2 bytes of header */, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

//                 memcpy(_payload, new_msg.data, new_msg.data_length);
//                 int err = sendto(sock, _payload, new_msg.data_length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//                 if (err < 0) {
//                     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                     break;
//                 }
//             } else {

//             }

//             // static const char *payload = "Alive!";
//             // char _payload[30] = { 0 };
//             // _payload[0] = 0;
//             // _payload[1] = 3;
//             // sprintf(&_payload[2], "%s", payload);

//             // int err = sendto(sock, _payload, strlen(payload) + 2 /* 2 bytes of header */, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//             // if (err < 0) {
//             //     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//             //     break;
//             // }

//             // vTaskDelay(2000 / portTICK_PERIOD_MS);
//             // vTaskDelay(1 / portTICK_PERIOD_MS);
//         }

//         if (sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }
//     }
//     vTaskDelete(NULL);
// }

// void udp_component_init(QueueHandle_t tcp_sending_queue) {
//     tcp_sending_queue = tcp_sending_queue;

//     // ESP_LOGI(TAG, "udp_alive_task start 1");
//     // xTaskCreate(udp_alive_task, "udp_alive", 4096, NULL, 5, NULL);
//     // ESP_LOGI(TAG, "udp_alive_task start 3");
    
//     xTaskCreate(udp_client_send_task, "udp_client_sending", 4096, NULL, 5 /* priority */, NULL);
//     ESP_LOGI(TAG, "udp_client_send_task start 4");

//     xTaskCreate(udp_client_conter_task, "udp_client_conter", 4096, NULL, 5 /* priority */, NULL);
//     ESP_LOGI(TAG, "udp_client_send_task start 5");
// }



TCPItem_t new_msg2;
TCPItem_t receive_msg;


static void tcp_client(void *arg)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
// #if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
// #elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
//         struct sockaddr_storage dest_addr = { 0 };
//         ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
// #endif

        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        // if (sock < 0) {
        //     ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        //     break;
        // }

        if (sock >= 0) {
            ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

            int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            // if (err != 0) {
            //     ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            //     break;
            // }

            if (err == 0) {
                ESP_LOGI(TAG, "Successfully connected");

                while (1) {
                    if (xQueueReceive(_tcp_sending_queue, (void *)&new_msg2, 1) == pdTRUE)
                    {
                    // uin8_t _buffer[8] = {0}
                        // counter++;
                        // printf("new_msg.id: %d %d\n", new_msg.id[1], new_msg.id[0]);
                        // printf(" new_msg.data_length: %d\n", new_msg.data_length);
                        // printf("new_msg.data: %s", new_msg.data);
                        // char _payload[200] = {0};
                        // memcpy(&_payload[2], &new_msg.data, new_msg.data_length);
                        // int err = sendto(sock, _payload, new_msg.data_length + 2 /* 2 bytes of header */, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

                        // memcpy(_payload, new_msg.data, new_msg.data_length);

                        // memcpy(&_payload[2], &new_msg.data, new_msg.data_length);
                        int err = send(sock, &new_msg2.data, new_msg2.data_length, 0);
                        // int err = send(sock, "haha\n", strlen(payload), 0);
                        if (err < 0)
                        {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            break;
                        }
                    }
                    else
                    {
                    }

                    // int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT);
                    // Error occurred during receiving
                    if (len < 0) {
                        if (errno == 11) {

                        } else {
                            ESP_LOGE(TAG, "recv failed: errno %d", errno);
                            break;
                        }
                    }
                    // Data received
                    else {
                        if (len == 0) {
                            ESP_LOGI(TAG, "Received 0 bytes from %s:", host_ip);
                        } else {
                            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                            ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                            ESP_LOGI(TAG, "%s", rx_buffer);
                            if (rx_buffer[0] == 0xcc) {
                                if (rx_buffer[1] == 0x00) {
                                    receive_msg.data_length = len-2;
                                    memcpy(&receive_msg.data, &rx_buffer[2], receive_msg.data_length);
                                    if (xQueueSend(_tcp_receiving_queue, (void *)&receive_msg, 2) != pdTRUE)
                                    {
                                        printf("ERROR: could not put item on _tcp_receiving_queue queue.\n");
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                ESP_LOGW(TAG, "Socket unable to connect: errno %d", errno);
            }
        } else {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        }

        if (sock != -1) {
            ESP_LOGW(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}


void tcp_client_init(QueueHandle_t tcp_sending_queue, QueueHandle_t tcp_receiving_queue) {
    _tcp_sending_queue = tcp_sending_queue;
    _tcp_receiving_queue = tcp_receiving_queue;

    // xTaskCreate(udp_alive_task, "udp_alive", 4096, NULL, 5, NULL);
    // ESP_LOGI(TAG, "udp_alive_task start 3");
    
    xTaskCreate(tcp_client, "tcp_client_task", 4096, NULL, 5 /* priority */, NULL);
    ESP_LOGI(TAG, "tcp_client start");
}
