# ESP32 WiFi Manager

A minimal, responsive WiFi configuration manager for ESP32-C6 (and compatible ESP-IDF boards) based on Espressif v5.4.1.  
Provides a self-hosted web interface for scanning, connecting, and managing WiFi credentials — no firmware recompile required!

There will be no maintance on this project.

---

## Features

- Web-based configuration interface served directly by the ESP32
- Scan for available WiFi networks (STA mode)
- Connect to WiFi via browser — no hardcoded credentials required
- Automatic Access Point (APSTA) mode if no credentials are stored or connection fails
- Real-time status overview: WiFi mode, connection state, SSID, IP address
- Reset stored WiFi credentials via the web interface
- Credentials securely saved in NVS storage (see [Security](#security))
- Robust STA/APSTA switching with timeout logic for connection monitoring

---

## Quickstart

### 1. Clone and Configure

```sh
git clone https://github.com/Sedowan/ESP32_WIFI_Manager.git
cd WIFI_Manager
```

### 2. Install ESP-IDF

Follow the official [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

### 3. Dependencies

Ensure the following ESP-IDF components are available:

- **esp_http_server** — for serving the web interface
- **nvs_flash** — for persistent credential storage

---

### 4. Build and Flash

```sh
idf.py set-target esp32c6    # or your compatible ESP32 variant
idf.py menuconfig           # optional project configuration
idf.py build
idf.py flash monitor
```

---

## Usage Workflow

1. **First Boot or Reset:**
   - Device starts in **Access Point mode**  
   - Default AP: SSID `ESP32-AP`, Password `esp32pass`

2. **Connect and Configure:**
   - Join the AP WiFi with your phone/laptop
   - Open [`http://192.168.4.1`](http://192.168.4.1) in a browser

3. **Web Interface Functions:**
   - Scan for available networks
   - Enter SSID and password to connect
   - View live status: mode, IP, SSID, connection state
   - Reset WiFi credentials if needed

4. **Normal Operation:**
   - On successful STA connection, APSTA mode deactivates
   - Device switches to WiFi client mode and joins your network
   - If connection drops or fails repeatedly, fallback to AP mode occurs automatically

---

## Project Structure

- `main/wifi_manager.c/.h` — Handles WiFi connection logic, NVS credential storage, AP/STA switching, and provides HTTP API endpoints
- `main/web.c/.h` — Minimal HTML/JavaScript web interface served via embedded resources
- `main/main.c` — Application entry point initializing WiFi manager and web server

---

## Security Considerations

- Credentials stored in NVS are unencrypted by default (ESP-IDF behavior)
- For production, consider enabling:
  - **NVS Encryption**
  - **Flash Encryption**
  - **Secure Boot**
- The configuration web interface has no authentication (intended for initial setup only)
- You can extend security with:
  - Custom password protection for the web interface
  - APSTA access restrictions (e.g., MAC filtering)

---

## Troubleshooting

- **Can't find AP?** Ensure your device supports WPA2 AP mode, and SSID `ESP32-AP` is visible
- **Web interface unresponsive?** Check serial output for errors, clear browser cache
- **Web interface unresponsive?** Check if still connected to the Accesspoint

---

## License

MIT License.  
See [LICENSE](LICENSE) for full terms.

---

## Credits

- [Espressif ESP-IDF](https://github.com/espressif/esp-idf)
- [cJSON](https://github.com/DaveGamble/cJSON)
- [ChatGPT](https://chat.openai.com/)

---
