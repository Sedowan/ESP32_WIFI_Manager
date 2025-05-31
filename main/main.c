#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "wifi_manager.h"
#include "web.h"

// Tag used for logging throughout this file
static const char *TAG = "Wifi_Manager";

/**
 * @brief HTTP GET handler for the root ("/") URI.
 * Serves the main HTML page (index_html).
 */
static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, strlen(index_html));
    return ESP_OK;
}

/**
 * @brief Initializes and starts the HTTP server, registering all relevant URI handlers.
 * 
 * @return httpd_handle_t The handle of the started HTTP server (or NULL on failure).
 */
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    
    if (httpd_start(&server, &config) == ESP_OK) {

        // Handler for root page (serves the frontend HTML)
        httpd_uri_t root = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = root_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root);

        // Handler for WiFi scan (returns available WiFi networks as JSON)
        httpd_uri_t scan_uri = {
            .uri       = "/scan_wifi",
            .method    = HTTP_GET,
            .handler   = wifi_manager_scan_get_wifi_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &scan_uri);

        // Handler for WiFi credentials POST (connects to selected network)
        httpd_uri_t wifi_post = {
            .uri      = "/wifi",
            .method   = HTTP_POST,
            .handler  = wifi_manager_post_wifi_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &wifi_post);

        // Handler to reset WiFi credentials (deletes stored SSID/password)
        httpd_uri_t wifi_reset = {
            .uri      = "/wifi_reset",
            .method   = HTTP_POST,
            .handler  = wifi_manager_post_wifi_reset_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &wifi_reset);
        
        // Handler for querying current WiFi status (returns JSON)
        httpd_uri_t wifi_status = {
            .uri      = "/wifi_status",
            .method   = HTTP_GET,
            .handler  = wifi_manager_get_wifi_status_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &wifi_status);

        ESP_LOGI(TAG, "HTTP server started.");
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP server.");
    }
    return server;
}

/**
 * @brief Main application entry point.
 * Initializes NVS, networking stack, WiFi manager, and starts the web server.
 */
void app_main(void)
{
    // Initialize NVS (non-volatile storage)
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize TCP/IP networking stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop (required for WiFi and networking)
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Initialize WiFi (starts in STA or AP mode depending on saved credentials)
    wifi_manager_init();

    // Start the web server and register all URI handlers
    start_webserver();

    ESP_LOGI(TAG, "Web server is running!");
}
