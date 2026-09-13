# Arduino IDE Setup & Flashing Guide

Follow this step-by-step guide to compile and flash the drone firmware onto your ESP32 board using Arduino IDE 2.x.

---

## 1. Install Arduino IDE
Download and install the latest **Arduino IDE 2.x** from the official site: [arduino.cc](https://www.arduino.cc/en/software).

---

## 2. Install ESP32 Board Package

1. Open Arduino IDE.
2. Navigate to **File > Preferences** (or `Ctrl + ,`).
3. In **Additional Boards Manager URLs**, paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click **OK**.
5. Go to **Tools > Board > Boards Manager...** (left sidebar).
6. Search for `esp32` by **Espressif Systems** and click **Install**.

---

## 3. Install Required Libraries

Go to **Tools > Manage Libraries...** (or `Ctrl + Shift + I`) and install the following libraries:

1. **ESPAsyncWebServer** by *me-no-dev* / *lacamera*
2. **AsyncTCP** by *me-no-dev* / *dvarrel*
3. **ArduinoJson** by *Benoît Blanchon* (Version 6.x or 7.x)
4. **Adafruit MPU6050** by *Adafruit*
5. **Adafruit Unified Sensor** by *Adafruit*
6. **Adafruit BusIO** (auto-installs with Adafruit MPU6050)

> [!TIP]
> If `ESPAsyncWebServer` or `AsyncTCP` are not found in the Library Manager in your IDE version, download their ZIP files directly from GitHub:
> - [AsyncTCP GitHub](https://github.com/dvarrel/AsyncTCP)
> - [ESPAsyncWebServer GitHub](https://github.com/lacamera/ESPAsyncWebServer)
>
> Then install via **Sketch > Include Library > Add .ZIP Library...**.

---

## 4. Board & Flashing Settings

In the **Tools** menu, configure your board parameters:

| Setting | Recommended Value |
|:---|:---|
| **Board** | `ESP32 Dev Module` (or `DOIT ESP32 DEVKIT V1`) |
| **CPU Frequency** | `240MHz (WiFi/BT)` |
| **Flash Frequency** | `80MHz` |
| **Flash Mode** | `QIO` |
| **Partition Scheme** | `Default 4MB with spiffs (1.2MB APP / 1.5MB SPIFFS)` |
| **Upload Speed** | `921600` (or `115200` if upload fails) |
| **Port** | Select the COM port corresponding to your connected ESP32 |

---

## 5. Uploading the Sketch

1. Open `firmware/esp32_drone_controller/esp32_drone_controller.ino`.
2. Connect your ESP32 to your PC via a high-quality micro-USB / USB-C data cable.
3. Click the **Upload** arrow button (`Ctrl + U`).
4. If your ESP32 gets stuck at `Connecting........_____.....`:
   - Press and hold the **BOOT** button on the ESP32 until the flashing progress percentage begins.
5. Once `Leaving... Hard resetting via RTS pin...` appears, the upload is complete!

---

## 6. Verifying Operation via Serial Monitor

1. Open **Tools > Serial Monitor** (`Ctrl + Shift + M`).
2. Set the baud rate to **115200 baud**.
3. Press the **EN / RESET** button on the ESP32.
4. You should see:
   ```
   ===========================================
     Web-Based Drone Control System (ESP32)
     Firmware Initializing...
   ===========================================
   [IMU] MPU-6050 connected & configured successfully.
   [WIFI] Access Point Created!
     SSID: ESP32-Drone-Control
     IP Address: 192.168.4.1
   [HTTP] AsyncWebServer & WebSocket listener running.
   [SYSTEM] Ready! Connect to Wi-Fi and open GCS in browser.
   ```
