#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "wifi_manager.h"
#include "web.h"

static const char* TAG = "Main";

/**
 * @brief Main application entry point. Initializes NVS, networking, starts WiFi manager and web server.
 */
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Start the WiFi manager state machine (handles STA/AP logic)
    wifi_manager_start_main_task();

    // Start the web server and register URI handlers
    web_start_server();

    ESP_LOGI("main", "Web server is running!");
}
