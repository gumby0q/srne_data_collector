
/* Uart Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/queue.h"

#include "../components/wifi/wifi.h"
#include "../components/networking/tcp_client_v4.h"


// #define CONFIG_ECHO_UART_BAUD_RATE 115200
// #define CONFIG_ECHO_UART_RXD 22
// #define CONFIG_ECHO_UART_TXD 23
// #define CONFIG_ECHO_UART_RTS 18
// #define CONFIG_ECHO_UART_PORT_NUM 2

#define RS485_UART_BAUD_RATE 9600
#define RS485_UART_RXD 22
#define RS485_UART_TXD 23
#define RS485_UART_RTS 18
#define RS485_UART_PORT_NUM 2


/**
 * This is a example which echos any data it receives on UART back to the sender using RS485 interface in half duplex mode.
*/
#define TAG "RS485_APP"

// Note: Some pins on target chip cannot be assigned for UART communication.
// Please refer to documentation for selected board and target to configure pins using Kconfig.
#define ECHO_TEST_TXD   (RS485_UART_TXD)
#define ECHO_TEST_RXD   (RS485_UART_RXD)

// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define ECHO_TEST_RTS   (RS485_UART_RTS)

// CTS is not used in RS485 Half-Duplex Mode
#define ECHO_TEST_CTS   (UART_PIN_NO_CHANGE)

#define BUF_SIZE        (127)
#define BAUD_RATE       (RS485_UART_BAUD_RATE)

// Read packet timeout
#define PACKET_READ_TICS        (100 / portTICK_PERIOD_MS)
#define ECHO_TASK_STACK_SIZE    (2048)
#define ECHO_TASK_PRIO          (10)
#define ECHO_UART_PORT          (RS485_UART_PORT_NUM)

// Timeout threshold for UART = number of symbols (~10 tics) with unchanged state on receive pin
#define ECHO_READ_TOUT          (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks


extern volatile uint8_t wifi_connected_flag;
extern uint8_t mac_base[6];

/* ------------------------------------------------------->>> */
static const int tcp_sending_queue_len = 5;     // Size of msg_queue
QueueHandle_t tcp_sending_queue;
static const int tcp_receiving_queue_len = 5;     // Size of msg_queue
QueueHandle_t tcp_receiving_queue;
// tcp_sending_queue = xQueueCreate(tcp_sending_queue_len, sizeof(TCPItem_t));
/* -------------------------------------------------------<<< */
// tcp_sending_queue = xQueueCreate(tcp_sending_queue_len, sizeof(Message_t));
//         if (xQueueReceive(msg_queue, (void *)&rcv_msg, 0) == pdTRUE)
//         {
//             printf("rcv_msg.body: %s", rcv_msg.body);
//             printf(" rcv_msg.count: %d\n", rcv_msg.count);
//         }
//         // Send integer to other task via queue
//         if (xQueueSend(delay_queue, (void *)&led_delay, 10) != pdTRUE)
//         {
//             printf("ERROR: Could not put item on delay queue.\n");
//         }

// static void echo_send(const int port, const char* str, uint8_t length)
// {
//     if (uart_write_bytes(port, str, length) != length) {
//         ESP_LOGE(TAG, "Send data critical failure.");
//         // add your code to handle sending failure here
//         abort();
//     }
// }


TCPItem_t receive_msg_rs;

static void rs485_send(const int uart_num, uint8_t * data, uint8_t length)
{
    if (uart_write_bytes(uart_num, data, length) != length) {
        ESP_LOGE(TAG, "Send data critical failure.");
        // add your code to handle sending failure here
        // abort();
    }
}

volatile uint8_t timeoutFlag = 0;

static void uart_send_task(void *arg)
{
    const int uart_num = ECHO_UART_PORT;

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    while (1)
    {
        // rs485_send(uart_num, (uint8_t * )"\r\n Hi there \r\n", 14);
        // rs485_send(uart_num, (uint8_t * )"Hilol", 14);
        // rs485_send(uart_num, (uint8_t * )"Hilol", 5);
        
        // vTaskDelay(2000 / portTICK_PERIOD_MS);
        if (xQueueReceive(tcp_receiving_queue, (void *)&receive_msg_rs, 1) == pdTRUE)
        {
            uint8_t buff[256] = {0};
            memcpy(buff, &receive_msg_rs.data, receive_msg_rs.data_length);
            rs485_send(uart_num, buff, receive_msg_rs.data_length);
        }
    }
    vTaskDelete(NULL);
}

TCPItem_t new_msg;
char message_data[350];
char _data[256];

// An example of echo test with hardware flow control on UART
static void echo_task(void *arg)
{
    const int uart_num = ECHO_UART_PORT;
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "Start RS485 application test and configure UART.");

    // Install UART driver (we don't need an event queue here)
    // In this example we don't even use a buffer for sending data.
    ESP_ERROR_CHECK(uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_LOGI(TAG, "UART set pins, mode and install driver.");

    // Set UART pins as per KConfig settings
    ESP_ERROR_CHECK(uart_set_pin(uart_num, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Set RS485 half duplex mode
    ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));

    // Set read timeout of UART TOUT feature
    ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, ECHO_READ_TOUT));

    // Allocate buffers for UART
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);

    ESP_LOGI(TAG, "UART start recieve loop.\r\n");
    printf("Hi im here \n");
    // echo_send(uart_num, "Start RS485 UART test.\r\n", 24);


    while(1) {
        //Read data from UART
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, PACKET_READ_TICS);

        //Write data back to UART
        if (len > 0) {
            // echo_send(uart_num, "\r\n", 2);
            // char prefix[] = "RS485 Received: [";
            // echo_send(uart_num, prefix, (sizeof(prefix) - 1));
            // ESP_LOGI(TAG, "Received %u bytes:", len);
            // printf("[ ");
            // for (int i = 0; i < len; i++) {
            //     printf("0x%.2X ", (uint8_t)data[i]);
            //     echo_send(uart_num, (const char*)&data[i], 1);
            //     // Add a Newline character if you get a return charater from paste (Paste tests multibyte receipt/buffer)
            //     if (data[i] == '\r') {
            //         echo_send(uart_num, "\n", 1);
            //     }
            // }

            ESP_LOGI(TAG, "Received %u bytes:", len);
            printf("[ ");
            for (int i = 0; i < len; i++) {
                printf("0x%.2X ", (uint8_t)data[i]);

                sprintf(&_data[i*2], "%.2X", (uint8_t)data[i]);
            }
            printf("] \n");

            // for (int i = 0; i < len; i++) {
            // }
                // printf("%s", _data);
            printf("\n");
            // echo_send(uart_num, "]\r\n", 3);



            if (wifi_connected_flag) {

/* 
            int len = sprintf(message_data, "{\
\"uuid\": \"%02x%02x%02x%02x%02x%02x\",\
\"type\": \"CONNECTION_DATA\"\
}", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
 */
                /* {
                "uuid": "some id",
                "type": "CHECK_KEY",
                "payload": {
                    "key": "some key"
                }
                } */

                int len = sprintf(message_data, "{\
\"uuid\": \"%02x%02x%02x%02x%02x%02x\",\
\"type\": \"RS485_RECEIVE\",\
\"payload\": {\"data\": \"%s\"}}", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5], _data);

                new_msg.data_length = len;
                memcpy(&new_msg.data, message_data, new_msg.data_length);

                if (xQueueSend(tcp_sending_queue, (void *)&new_msg, 2) != pdTRUE)
                {
                    printf("ERROR: uart_module could not put item on delay queue.\n");
                }
            }
        } else {
            // Echo a "." to show we are alive while we wait for input
            // echo_send(uart_num, ".", 1);
            // ESP_ERROR_CHECK(uart_wait_tx_done(uart_num, 10));
        }
    }
    vTaskDelete(NULL);
}


void app_main(void)
{
    tcp_sending_queue = xQueueCreate(tcp_sending_queue_len, sizeof(TCPItem_t));
    tcp_receiving_queue = xQueueCreate(tcp_receiving_queue_len, sizeof(TCPItem_t));
    

    wifi_init();
    ESP_LOGI("main", "wifi_init() done");

    tcp_client_init(tcp_sending_queue, tcp_receiving_queue);
    ESP_LOGI("main", "tcp_client_init() done");

    //A uart read/write example without event queue;
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, ECHO_TASK_PRIO, NULL);
    xTaskCreate(uart_send_task, "uart_send_task", ECHO_TASK_STACK_SIZE, NULL, ECHO_TASK_PRIO, NULL);
}

