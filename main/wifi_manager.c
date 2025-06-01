#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file wifi_manager.h
 * @brief WiFi management interface for ESP32 projects with web-based configuration.
 *
 * This module provides functions for WiFi credential storage,
 * switching between Station and Access Point mode, and
 * HTTP handler endpoints for a browser-based UI.
 */

/**
 * @brief Initializes mDNS with the given hostname for network discovery.
 * @param hostname The mDNS hostname (e.g. "esp32").
 */
void wifi_manager_mdns_init(const char* hostname);

/**
 * @brief Initializes the WiFi subsystem and internal state.
 *
 * Usually called from the WiFi manager's main task,
 * not required to call manually.
 */
void wifi_manager_init(void);

/**
 * @brief Starts the WiFi manager main state machine as a FreeRTOS task.
 *
 * Handles automatic switching between STA and AP mode,
 * and manages timeouts and client connection checks.
 */
void wifi_manager_start_main_task(void);

/**
 * @brief Starts the ESP32 in Access Point (AP) mode with default credentials.
 *
 * AP mode provides a configuration portal for WiFi setup.
 */
void wifi_manager_start_ap(void);

/**
 * @brief Stops the Access Point mode.
 */
void wifi_manager_stop_ap(void);

/**
 * @brief Connects to a WiFi network in Station (STA) mode with given credentials.
 * @param ssid     SSID of the WiFi network.
 * @param password Password of the WiFi network.
 */
void wifi_manager_connect_sta(const char *ssid, const char *password);

/**
 * @brief Checks if WiFi credentials (SSID and password) exist in NVS and loads them.
 * @return true if credentials are present, false otherwise.
 */
bool wifi_manager_wifi_credentials_exist(void);

/**
 * @brief Saves the provided SSID and password into NVS (persistent storage).
 * @param ssid     SSID to save.
 * @param password Password to save.
 */
void wifi_manager_save_wifi_credentials(const char* ssid, const char* password);

/**
 * @brief Deletes the stored WiFi credentials (SSID and password) from NVS.
 */
void wifi_manager_delete_wifi_credentials(void);

/**
 * @brief Attempts to connect to a WiFi network to test the given credentials.
 * @param ssid     SSID to test.
 * @param password Password to test.
 * @return ESP_OK on successful connection, ESP_FAIL otherwise.
 */
esp_err_t wifi_manager_wifi_connect_test(const char *ssid, const char *password);

/**
 * @brief HTTP GET handler: Scans for available WiFi networks and returns a JSON array.
 * Endpoint: /wifi_scan
 */
esp_err_t wifi_manager_scan_get_wifi_handler(httpd_req_t *req);

/**
 * @brief HTTP POST handler: Receives WiFi credentials from the web form and attempts connection.
 * Endpoint: /wifi
 */
esp_err_t wifi_manager_post_wifi_handler(httpd_req_t *req);

/**
 * @brief HTTP POST handler: Deletes stored WiFi credentials (resets WiFi config).
 * Endpoint: /wifi_reset
 */
esp_err_t wifi_manager_post_wifi_reset_handler(httpd_req_t *req);

/**
 * @brief HTTP GET handler: Returns the current WiFi status as a JSON object.
 * Endpoint: /wifi_status
 */
esp_err_t wifi_manager_get_wifi_status_handler(httpd_req_t *req);

/**
 * @brief Buffers for storing loaded WiFi credentials internally.
 * These are updated by wifi_manager_wifi_credentials_exist().
 */
extern char saved_ssid[];
extern char saved_pass[];

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
