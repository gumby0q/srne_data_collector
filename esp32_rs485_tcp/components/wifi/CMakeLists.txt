set(srcs  "wifi.c")

idf_component_register(REQUIRES "esp_wifi" "nvs_flash"
                       SRCS "${srcs}"
                       INCLUDE_DIRS ".")
