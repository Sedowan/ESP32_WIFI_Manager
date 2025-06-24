#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

// Indicates if the ESP32-C6 is connected to a Wi-Fi network in STA mode
extern bool wifi_manager_sta_connected;

/**
 * @brief Initializes the WiFi subsystem and starts AP or STA mode,
 *        depending on whether credentials are found in NVS.
 */
void wifi_manager_init(void);

/**
 * @brief HTTP GET handler: Scans for WiFi networks and returns a JSON array.
 *
 * Endpoint: /wifi_scan (method: GET)
 *
 * @param req HTTP request pointer
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t wifi_manager_scan_get_wifi_handler(httpd_req_t *req);

/**
 * @brief HTTP POST handler: Receives WiFi credentials from the web form and attempts to connect.
 *
 * Endpoint: /wifi (method: POST)
 * Form format: application/x-www-form-urlencoded (e.g. ssid=XYZ&password=ABC)
 *
 * @param req HTTP request pointer
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t wifi_manager_post_wifi_handler(httpd_req_t *req);

/**
 * @brief Connects to a WiFi network in STA mode using the provided credentials.
 *
 * @param ssid SSID of the WiFi network
 * @param password Password of the WiFi network
 */
void wifi_manager_connect_sta(const char *ssid, const char *password);

/**
 * @brief Starts the ESP32 in Access Point (AP) mode with default credentials.
 */
void wifi_manager_start_ap(void);

/**
 * @brief Checks if valid WiFi credentials are present in NVS and loads them into RAM.
 * 
 * This function verifies if SSID and password exist in the non-volatile storage
 * and populates the global variables `saved_ssid` and `saved_pass`.
 *
 * @return true if valid credentials were found, false otherwise
 */
bool wifi_manager_wifi_credentials_exist(void);

/**
 * @brief Saves the provided SSID and password into NVS.
 *
 * @param ssid SSID to save
 * @param password Password to save
 */
void wifi_manager_save_wifi_credentials(const char* ssid, const char* password);

/**
 * @brief Deletes the saved WiFi credentials (SSID and password) from NVS.
 */
void wifi_manager_delete_wifi_credentials(void);

/**
 * @brief Starts the main WiFi Manager task which runs indefinitely.
 * 
 * This FreeRTOS task handles automatic switching between:
 * - STA mode (Station/client) if credentials are present
 * - APSTA mode (Access Point/STA) for fallback configuration
 * 
 * It monitors connection state, handles timeouts, and ensures a device is always reachable.
 */
void wifi_manager_start_main_task(void);

/**
 * @brief Attempts to connect to a WiFi network to test the given credentials.
 *
 * @param ssid SSID to test
 * @param password Password to test
 * @return ESP_OK on successful connection, ESP_FAIL otherwise
 */
esp_err_t wifi_manager_wifi_connect_test(const char *ssid, const char *password);

/**
 * @brief HTTP POST handler: Deletes stored WiFi credentials (resets WiFi settings).
 *
 * Endpoint: /wifi_reset (method: POST)
 *
 * @param req HTTP request pointer
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t wifi_manager_post_wifi_reset_handler(httpd_req_t *req);

/**
 * @brief HTTP GET handler: Returns the current WiFi status as a JSON object.
 *
 * Endpoint: /wifi_status (method: GET)
 *
 * @param req HTTP request pointer
 * @return ESP_OK
 */
esp_err_t wifi_manager_get_wifi_status_handler(httpd_req_t *req);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H