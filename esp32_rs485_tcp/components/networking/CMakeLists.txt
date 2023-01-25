 set(srcs  "tcp_client_v4.c")

idf_component_register(SRCS "${srcs}"
                       REQUIRES esp_netif
                       INCLUDE_DIRS ".")
