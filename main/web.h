#ifndef WEB_H
#define WEB_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts the HTTP server and registers all URI handlers, including root and WiFi manager endpoints.
 */
httpd_handle_t web_start_server(void);

/**
 * @brief HTML/JS/CSS frontend embedded as a C string array.
 */
extern const char index_html[];

#ifdef __cplusplus
}
#endif

#endif // WEB_H
