#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "mdns.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

#define WIFI_MANAGER_STA_ATTEMPT_DURATION_MS     (3 * 60 * 1000) // 3 minutes in AP-Mode
#define WIFI_MANAGER_AP_IDLE_TIMEOUT_MS          (2 * 60 * 1000) // 2 minutes in STA-Mode
#define WIFI_MANAGER_AP_CLIENT_CHECK_INTERVAL_MS (5 * 1000)      // 5 seconds interval to check clients connected in AP-Mode
#define WIFI_MANAGER_STA_RECONNECT_TIMEOUT_SEC   (60)            // 60 seconds to check if still connected while in STA-Mode   
#define WIFI_NAMESPACE "wifi_creds"
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

static const char* TAG = "wifi_manager";
char saved_ssid[MAX_SSID_LEN];
char saved_pass[MAX_PASS_LEN];

// Global variables accessible by other modules
bool wifi_manager_sta_connected = false;

/**
 * @brief Checks if WiFi credentials are stored in NVS.
 */
bool wifi_manager_wifi_credentials_exist(void) {
    nvs_handle_t nvs;
    size_t len;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READONLY, &nvs);
    if (err != ESP_OK) return false;

    len = sizeof(saved_ssid);
    err = nvs_get_str(nvs, "wifi_ssid", saved_ssid, &len);
    if (err != ESP_OK) {
        nvs_close(nvs);
        return false;
    }

    len = sizeof(saved_pass);
    err = nvs_get_str(nvs, "wifi_pass", saved_pass, &len);
    nvs_close(nvs);
    return err == ESP_OK;
}

/**
 * @brief Saves WiFi credentials (SSID and password) into NVS.
 */
void wifi_manager_save_wifi_credentials(const char* ssid, const char* password) {
    nvs_handle_t nvs;
    if (nvs_open(WIFI_NAMESPACE, NVS_READWRITE, &nvs) == ESP_OK) {
        nvs_set_str(nvs, "wifi_ssid", ssid);
        nvs_set_str(nvs, "wifi_pass", password);
        nvs_commit(nvs);
        nvs_close(nvs);
        ESP_LOGI(TAG, "WiFi credentials saved: SSID='%s'", ssid);
    } else {
        ESP_LOGE(TAG, "Failed to save WiFi credentials.");
    }
}

/**
 * @brief Deletes WiFi credentials from NVS.
 */
void wifi_manager_delete_wifi_credentials(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS for deleting credentials: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_erase_key(nvs_handle, "wifi_ssid");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Failed to erase SSID: %s", esp_err_to_name(err));
    }

    err = nvs_erase_key(nvs_handle, "wifi_pass");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Failed to erase password: %s", esp_err_to_name(err));
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Commit failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "WiFi credentials deleted from NVS.");
    }

    nvs_close(nvs_handle);
}

/**
 * @brief Connects the ESP32 to a WiFi network in STA (client) mode.
 */
void wifi_manager_connect_sta(const char *ssid, const char *password) {
    
    esp_wifi_stop();       // Safe to call, even when not running
    esp_wifi_deinit();     // Safe to call, resets WIFI-drivers
    
    
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();

    ESP_LOGI(TAG, "Started STA mode with SSID: %s", ssid);
}

/**
 * @brief Starts the ESP32 in Access Point (AP) mode.
 */
void wifi_manager_start_ap(void) {
    
    esp_wifi_stop();       // Safe to call, even when not running
    esp_wifi_deinit();     // Safe to call, resets WIFI-drivers
    
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32-AP",
            .ssid_len = 8,
            .password = "esp32pass",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();
    ESP_LOGI(TAG, "Started AP mode: SSID: ESP32-AP, PASS: esp32pass");
}

/**
 * @brief Stops the Access Point mode.
 */
void wifi_manager_stop_ap(void) {
    esp_wifi_stop();
    ESP_LOGI(TAG, "Stopped AP mode.");
}

/**
 * @brief Initializes mDNS for network discovery.
 */
void wifi_manager_mdns_init(const char* hostname) {
    if (mdns_init() == ESP_OK) {
        mdns_hostname_set(hostname);
        mdns_instance_name_set("ESP32 Web Server");
        ESP_LOGI(TAG, "mDNS started: %s.local", hostname);
    } else {
        ESP_LOGE(TAG, "Failed to initialize mDNS.");
    }
}

/**
 * @brief The main WiFi state/task machine. Handles STA/AP switching and all timing logic.
 * Keeps AP mode active as long as a client is connected.
 */
static void wifi_manager_main_task(void *pvParameters) {
    while (1) {
        if (wifi_manager_wifi_credentials_exist()) {
            wifi_manager_connect_sta(saved_ssid, saved_pass);

            uint32_t elapsed_sta = 0;
            bool connected = false;
            while (elapsed_sta < WIFI_MANAGER_STA_ATTEMPT_DURATION_MS) {
                wifi_ap_record_t info;
                if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
                    ESP_LOGI(TAG, "Connected to WiFi. Remaining in STA mode.");
                    connected = true;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
                elapsed_sta += 1000;
            }

            if (connected) {
                // Monitor STA connection: restart cycle if disconnected for too long
                int lost_seconds = 0;
                const int RECONNECT_TIMEOUT = WIFI_MANAGER_STA_RECONNECT_TIMEOUT_SEC;
                while (1) {
                    wifi_ap_record_t info;
                    if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
                        // Still connected
                        lost_seconds = 0;
                    } else {
                        // Connection lost!
                        lost_seconds++;
                        ESP_LOGW(TAG, "WiFi connection lost for %d seconds.", lost_seconds);
                        if (lost_seconds >= RECONNECT_TIMEOUT) {
                            ESP_LOGW(TAG, "Connection lost > %d seconds. Restarting AP/STA cycle...", RECONNECT_TIMEOUT);
                            esp_wifi_disconnect();
                            esp_wifi_stop();
                            esp_wifi_deinit();
                            break; // Return to main state machine: try STA again, then AP, etc.
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
                }
                continue; // Loop back to main state machine
            }

            ESP_LOGW(TAG, "STA connection failed. Switching to AP mode for user intervention.");
            wifi_manager_start_ap();

            uint32_t ap_idle_time = 0;
            while (ap_idle_time < WIFI_MANAGER_AP_IDLE_TIMEOUT_MS) {
                wifi_sta_list_t sta_list;
                esp_wifi_ap_get_sta_list(&sta_list);

                if (sta_list.num > 0) {
                    ESP_LOGI(TAG, "AP client connected, pausing AP idle timer.");
                    ap_idle_time = 0;
                } else {
                    ap_idle_time += WIFI_MANAGER_AP_CLIENT_CHECK_INTERVAL_MS;
                }
                vTaskDelay(pdMS_TO_TICKS(WIFI_MANAGER_AP_CLIENT_CHECK_INTERVAL_MS));
            }

            ESP_LOGI(TAG, "No AP clients for %d seconds, stopping AP and retrying STA.", WIFI_MANAGER_AP_IDLE_TIMEOUT_MS / 1000);
            wifi_manager_stop_ap();

        } else {
            ESP_LOGI(TAG, "No WiFi credentials stored, starting AP mode only.");
            wifi_manager_start_ap();
            vTaskDelay(pdMS_TO_TICKS(60 * 1000));
        }
    }
}

void wifi_manager_start_main_task(void) {
    xTaskCreate(&wifi_manager_main_task, "wifi_manager_main_task", 4096, NULL, 5, NULL);
}

esp_err_t wifi_manager_scan_get_wifi_handler(httpd_req_t *req) {
    uint16_t ap_num = 0;
    wifi_ap_record_t *ap_records = NULL;

    esp_wifi_scan_start(NULL, true);
    esp_wifi_scan_get_ap_num(&ap_num);

    ap_records = malloc(sizeof(wifi_ap_record_t) * ap_num);
    if (!ap_records) return ESP_ERR_NO_MEM;

    esp_wifi_scan_get_ap_records(&ap_num, ap_records);

    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < ap_num; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "ssid", (const char*)ap_records[i].ssid);
        cJSON_AddNumberToObject(item, "rssi", ap_records[i].rssi);
        cJSON_AddBoolToObject(item, "secure", ap_records[i].authmode != WIFI_AUTH_OPEN);
        cJSON_AddItemToArray(root, item);
    }

    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));

    free(ap_records);
    cJSON_Delete(root);
    free((void*)json_str);
    return ESP_OK;
}

esp_err_t wifi_manager_wifi_connect_test(const char *ssid, const char *password) {
    ESP_LOGI(TAG, "Testing connection to SSID: %s", ssid);

    wifi_manager_connect_sta(ssid, password);
    vTaskDelay(pdMS_TO_TICKS(3000)); // Wait 3 seconds for connection

    wifi_ap_record_t info;
    if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
        ESP_LOGI(TAG, "Connection successful");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Connection failed");
        esp_wifi_stop();
        return ESP_FAIL;
    }
}

esp_err_t wifi_manager_post_wifi_handler(httpd_req_t *req) {
    char buf[256];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            return ESP_FAIL;
        }
        remaining -= ret;
        buf[ret] = '\0';
    }

    char ssid[64] = {0};
    char password[64] = {0};
    char *ssid_ptr = strstr(buf, "ssid=");
    char *pass_ptr = strstr(buf, "password=");

    if (ssid_ptr) {
        ssid_ptr += strlen("ssid=");
        char *end = strchr(ssid_ptr, '&');
        if (end) *end = '\0';
        strncpy(ssid, ssid_ptr, sizeof(ssid) - 1);
    }

    if (pass_ptr) {
        pass_ptr += strlen("password=");
        strncpy(password, pass_ptr, sizeof(password) - 1);
    }

    ESP_LOGI(TAG, "Received: SSID='%s', Password='%s'", ssid, password);

    esp_err_t result = wifi_manager_wifi_connect_test(ssid, password);

    if (result == ESP_OK) {
        wifi_manager_save_wifi_credentials(ssid, password);
        httpd_resp_send(req, "Connection successful. Credentials saved.", HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_send(req, "Connection failed. Please check credentials.", HTTPD_RESP_USE_STRLEN);
    }

    return result;
}

esp_err_t wifi_manager_post_wifi_reset_handler(httpd_req_t *req)
{
    wifi_manager_delete_wifi_credentials();
    ESP_LOGI(TAG, "WiFi credentials reset via HTTP handler.");
    httpd_resp_send(req, "Credentials deleted. Please reboot.", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t wifi_manager_get_wifi_status_handler(httpd_req_t *req) {
    wifi_mode_t mode;
    wifi_config_t conf;
    char ip[16] = "0.0.0.0";
    bool connected = false;

    esp_wifi_get_mode(&mode);
    esp_wifi_get_config(ESP_IF_WIFI_STA, &conf);

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        snprintf(ip, sizeof(ip), IPSTR, IP2STR(&ip_info.ip));
        connected = ip_info.ip.addr != 0;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mode", mode == WIFI_MODE_STA ? "station" : "accesspoint");
    cJSON_AddStringToObject(root, "ssid", (const char *)conf.sta.ssid);
    cJSON_AddStringToObject(root, "ip", ip);
    cJSON_AddBoolToObject(root, "connected", connected);
    const char *response = cJSON_Print(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    free((void *)response);
    cJSON_Delete(root);

    return ESP_OK;
}
