#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "freertos/queue.h"
#include <stdio.h>


struct TCPItem {
  uint8_t data[256];
  uint16_t data_length;
};
typedef struct TCPItem TCPItem_t;


void tcp_client_init(QueueHandle_t tcp_sending_queue);

#endif
