# ESP32 Pinout Reference & GPIO Allocation

This document explains the pin allocation for the flight controller, noting hardware timers, ADC channels, and boot strapping pins to avoid conflicts.

---

## 1. Pin Assignment Table

| ESP32 Pin | Function | Peripheral | Notes |
|:---|:---|:---|:---|
| **GPIO 25** | Motor 1 (FL - Front Left) | LEDC PWM Ch 0 | Safe output. No boot strapping conflicts. |
| **GPIO 26** | Motor 2 (FR - Front Right) | LEDC PWM Ch 1 | Safe output. No boot strapping conflicts. |
| **GPIO 32** | Motor 3 (RL - Rear Left) | LEDC PWM Ch 2 | Safe output. |
| **GPIO 33** | Motor 4 (RR - Rear Right) | LEDC PWM Ch 3 | Safe output. |
| **GPIO 21** | I2C SDA | Hardware I2C 0 | Connected to MPU-6050 SDA |
| **GPIO 22** | I2C SCL | Hardware I2C 0 | Connected to MPU-6050 SCL |
| **GPIO 34** | Battery Voltage Sense | ADC1 Channel 6 | Input-only pin. Perfect for analog sensor reading. |
| **GPIO 2** | Status LED | Internal LED | Blinks on Wi-Fi connection / Armed status |

---

## 2. Strapping Pins & Warnings

> [!CAUTION]
> Avoid using the following pins for motor outputs or external pull-ups:
> - **GPIO 0**: Boot mode selector (pulling LOW enters download mode).
> - **GPIO 2**: Must be LOW or left floating during boot; connected to onboard LED.
> - **GPIO 12 (MTDI)**: If pulled HIGH at boot, flash voltage switches to 1.8V instead of 3.3V, bricking the boot cycle until reset.
> - **GPIO 15**: Boot message output control.
> - **GPIO 34–39**: Input-only pins (GPI); cannot produce PWM signals for motors.

Our pin mapping utilizes **GPIO 25, 26, 32, and 33**, which are completely free of strapping pin hazards and guarantee reliable bootup every time.
