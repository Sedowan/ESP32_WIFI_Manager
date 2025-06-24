idf_component_register(
    SRCS 
        "main.c"
        "web.c"
        "wifi_manager.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_http_server
        esp_wifi
        nvs_flash
        esp_netif
        esp_event
        json
)
