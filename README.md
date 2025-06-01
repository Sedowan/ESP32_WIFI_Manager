# ESP32 WiFi Manager

A minimal, responsive WiFi configuration manager for ESP32-C6 (and compatible ESP-IDF boards) based on Espressif v5.2.2.  
Provides a web interface for scanning, connecting, and managing WiFi credentials – all without recompiling firmware!

---

## Features

- Scan for available WiFi networks (STA mode)
- Connect to WiFi via web frontend (no hardcoded credentials)
- Automatic fallback to Access Point (AP) mode if no credentials are stored
- Reset (delete) WiFi credentials with one click
- View connection status, SSID, mode, IP address in real time
- Built-in web server serves a simple HTML/JavaScript frontend
- Credentials securely stored in NVS (see [Security](#security))
- Optional mDNS support for easy device discovery on the network

---

## Quickstart

### 1. Clone and configure

```sh
git clone https://github.com/Sedowan/WIFI_Manager.git
cd WIFI_Manager
```

### 2. Install ESP-IDF (if not done)

Follow the official [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

### 3. Add dependencies

Ensure you have the following ESP-IDF components enabled:
- **esp_http_server**
- **nvs_flash**
- **espressif/mdns** (for mDNS functionality, optional but recommended)

If you use an `idf_component.yml`, add:

```yaml
dependencies:
  espressif/mdns: "^1.0.2"
```

---

### 4. Build and Flash

```sh
idf.py set-target esp32c6    # or your ESP32 variant
idf.py menuconfig           # (optional) for project config
idf.py build
idf.py flash monitor
```

---

### 5. Usage

- On first boot (or after WiFi reset), the device starts in **Access Point mode** (SSID: `ESP32-AP`, Password: `esp32pass`).
- Connect to the AP, then open [`http://192.168.4.1`](http://192.168.4.1) in your browser.
- Scan for networks or manually enter SSID and password, then click **Connect**.
- On successful connection, the ESP32 will switch to STA mode and join your WiFi.
- The web UI shows status, mode, and allows you to reset credentials anytime.

---

## Security

- WiFi credentials are stored in NVS in plain text by default (ESP-IDF standard).
- For production, enable **NVS Encryption**, **Flash Encryption**, and/or **Secure Boot** in your ESP-IDF project.
- The web UI is intentionally simple and **not password-protected** (intended for initial device setup only).  
  If you need authentication, consider adding a password field or restricting the AP by MAC address.

---

## mDNS

- To access your ESP32 via hostname (e.g. `esp32.local`), mDNS support is included.
- Make sure the **espressif/mdns** component is available in your ESP-IDF setup.

---

## File Structure

- `main/wifi_manager.c/.h` — WiFi credential management logic
- `main/web.c/.h`          — Embedded web frontend (HTML/JS as C array)
- `main/main.c`            — Application entry point, HTTP server setup

---

## Troubleshooting

- **Can't connect to AP?** Double-check your phone/laptop supports WPA2 APs, and SSID/Password matches.
- **Can't find `/wifi_scan` or status doesn't update?** Clear browser cache, check ESP32 serial log for errors.
- **Build error about `mdns`?** Add `espressif/mdns` to your dependencies as shown above.

---

## License

MIT License.  
See [LICENSE](LICENSE) for details.

---

## Credits

- [Espressif ESP-IDF](https://github.com/espressif/esp-idf)
- [cJSON](https://github.com/DaveGamble/cJSON)
- [espressif/mdns](https://github.com/espressif/esp-protocols/tree/master/components/mdns)
- [ChatGPT](https://chatgpt.com/)
