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
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <title>ESP32 WiFi Configuration</title>\n"
"    <script>\n"
"        async function loadWiFiStatus() {\n"
"            try {\n"
"                const response = await fetch('/wifi_status');\n"
"                const status = await response.json();\n"
"                document.getElementById('status_mode').textContent = status.mode;\n"
"                document.getElementById('status_ssid').textContent = status.ssid || 'Not connected';\n"
"                document.getElementById('status_ip').textContent = status.ip || '-';\n"
"                document.getElementById('status_connected').textContent = status.connected ? 'Yes' : 'No';\n"
"            } catch (e) {\n"
"                console.error('Error loading status', e);\n"
"            }\n"
"        }\n"
"        async function loadNetworks() {\n"
"            try {\n"
"                const response = await fetch('/wifi_scan');\n"
"                const data = await response.json();\n"
"                const select = document.getElementById('ssid_select');\n"
"                select.innerHTML = '<option value=\"\">Enter SSID manually</option>';\n"
"                data.forEach(network => {\n"
"                    const option = document.createElement('option');\n"
"                    option.value = network;\n"
"                    option.text = network;\n"
"                    select.appendChild(option);\n"
"                });\n"
"            } catch (e) {\n"
"                console.error('Error loading networks', e);\n"
"            }\n"
"        }\n"
"        function updateSSIDField() {\n"
"            const select = document.getElementById('ssid_select');\n"
"            const ssidInput = document.getElementById('ssid_input');\n"
"            if (select.value !== '') {\n"
"                ssidInput.value = select.value;\n"
"            }\n"
"        }\n"
"        async function resetWiFi() {\n"
"            if (confirm('Do you really want to delete the saved WiFi credentials?')) {\n"
"                await fetch('/wifi_reset', { method: 'POST' });\n"
"                alert('Credentials deleted. The device will restart in Access Point mode.');\n"
"                location.reload();\n"
"            }\n"
"        }\n"
"        window.onload = function() {\n"
"            loadWiFiStatus();\n"
"            loadNetworks();\n"
"        };\n"
"    </script>\n"
"</head>\n"
"<body>\n"
"    <h1>ESP32 WiFi Configuration</h1>\n"
"    <fieldset>\n"
"        <legend>Status</legend>\n"
"        <p>Mode: <span id=\"status_mode\">-</span></p>\n"
"        <p>SSID: <span id=\"status_ssid\">-</span></p>\n"
"        <p>IP address: <span id=\"status_ip\">-</span></p>\n"
"        <p>Connected: <span id=\"status_connected\">-</span></p>\n"
"    </fieldset>\n"
"    <h2>Select or enter WiFi network</h2>\n"
"    <form method=\"POST\" action=\"/wifi\">\n"
"        <label for=\"ssid_select\">Detected networks:</label><br>\n"
"        <select id=\"ssid_select\" onchange=\"updateSSIDField()\"></select><br><br>\n"
"        <label for=\"ssid_input\">SSID:</label><br>\n"
"        <input type=\"text\" id=\"ssid_input\" name=\"ssid\"><br><br>\n"
"        <label for=\"password\">Password:</label><br>\n"
"        <input type=\"password\" name=\"password\"><br><br>\n"
"        <button type=\"submit\">Connect</button>\n"
"    </form>\n"
"    <br>\n"
"    <button onclick=\"resetWiFi()\">Reset credentials</button>\n"
"</body>\n"
"</html>\n"
"";
