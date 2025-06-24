#include "web.h"
#include "wifi_manager.h"
#include "esp_log.h"
#include <string.h>

/**
 * @brief HTTP GET handler for the root ("/") URI. Serves the main HTML page.
 */
static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, strlen(index_html));
    return ESP_OK;
}

/**
 * @brief Starts the HTTP server and registers all URI handlers, including WiFi manager endpoints.
 * Returns the server handle, or NULL if startup failed.
 */
httpd_handle_t web_start_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t scan_uri = { .uri = "/wifi_scan", .method = HTTP_GET, .handler = wifi_manager_scan_get_wifi_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &scan_uri);

        httpd_uri_t wifi_post = { .uri = "/wifi", .method = HTTP_POST, .handler = wifi_manager_post_wifi_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &wifi_post);

        httpd_uri_t wifi_reset = { .uri = "/wifi_reset", .method = HTTP_POST, .handler = wifi_manager_post_wifi_reset_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &wifi_reset);

        httpd_uri_t wifi_status = { .uri = "/wifi_status", .method = HTTP_GET, .handler = wifi_manager_get_wifi_status_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &wifi_status);

        ESP_LOGI("web", "HTTP server started.");
    } else {
        ESP_LOGE("web", "Failed to start HTTP server.");
    }
    return server;
}

const char index_html[] = "\n"
        "<!DOCTYPE html>"
            "<html>"
                "<head>"
                    "<meta charset='utf-8'>"
                        "<title>ESP32 WiFi/MQTT/Zigbee Config</title>"
                            "<style>body{font-family:sans-serif;margin:2em;}fieldset{margin-bottom:1em;}</style>"
                    "<script>"

                        "async function loadWiFiStatus() {"
                            "try {"
                                "const r=await fetch('/wifi_status');"
                                "const s=await r.json();"
                                "document.getElementById('status_connected').textContent=s.connected?'Yes':'No';"
                                "document.getElementById('status_mode').textContent=s.mode;"
                                "document.getElementById('status_ssid').textContent=s.ssid;"
                                "document.getElementById('status_ip').textContent=s.ip;"
                            "} catch (e) {"
                                "console.error('WiFi status error',e);"
                            "}"
                        "}"

                        "async function loadNetworks() {"
                            "try {"
                                "const r=await fetch('/wifi_scan');"
                                "const l=await r.json();"
                                "const s=document.getElementById('ssid_select');"
                                "s.innerHTML='';"
                                "l.forEach(n=>{"
                                    "const o=document.createElement('option');"
                                    "o.value=n.ssid;"
                                    "o.text=n.ssid;"
                                    "s.appendChild(o);"
                                    "}"
                                ");"
                            "} catch (e) {"
                                "console.error('Scan error',e);"
                            "}"
                        "}"

                        "function fillSSID() {"
                            "document.getElementById('ssid').value=document.getElementById('ssid_select').value;"
                        "}"

                        "window.onload=function() {"
                            "loadWiFiStatus();"
                            "loadNetworks();"
                            "setInterval(loadWiFiStatus,3000);"
                            "setInterval(loadNetworks, 3000);"
                        "};"

                    "</script>"
                "</head>"
                "<body>"
                    "<h1>ESP32 Status</h1>"
                    "<fieldset>"
                        "<legend>Sytem Status</legend>"
                        "<p>WIFI connected: <span id='status_connected'>-</span></p>"
                        "<p>WIFI mode: <span id='status_mode'>-</span></p>"
                        "<p>SSID: <span id='status_ssid'>-</span></p>"
                        "<p>IP Address: <span id='status_ip'>-</span></p>"
                    "</fieldset>"

                    "<h1>ESP32 Configuration</h1>"
                    "<fieldset>"
                        "<legend>WiFi Configuration</legend>"
                        "<form method='POST' action='/wifi'>"
                            "<p>SSID: <input id='ssid' name='ssid'><select id='ssid_select' onchange='fillSSID()'></select></p>"
                            "<p>Password: <input type='password' name='password'></p>"
                            "<p><input type='submit' value='Connect'></form>"
                        "</form></p>"

                        "<form method='POST' action='/wifi_reset'>"
                            "<button type='submit'>Reset WiFi</button>"
                        "</form>"
                    "</fieldset>"
                    
                "</body>"
            "</html>";