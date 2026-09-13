# Bill of Materials (BOM)

Comprehensive list of electronic and mechanical hardware components required to assemble the **Web-Based Drone Control System**.

---

## 1. Electronics & Microcontrollers

| Component | Specification / Model | Qty | Purpose | Est. Cost (USD / INR) |
|:---|:---|:---:|:---|:---|
| **Microcontroller** | ESP32 DevKit V1 (30-pin or 38-pin NodeMCU) | 1 | Flight Controller MCU, Wi-Fi Server, PWM generation | $4.00 / ₹350 |
| **IMU Sensor** | MPU-6050 (GY-521 Breakout) | 1 | 6-DOF Gyroscope + Accelerometer for attitude stabilization | $1.50 / ₹130 |
| **MOSFETs (Brushed)** | SI2302 / AO3400A / IRLML6344 N-Channel | 4 | High-speed PWM motor driver switches (brushed setup) | $1.00 / ₹80 |
| **Flyback Diodes** | 1N5819 Schottky Diodes | 4 | Inductive back-EMF spike suppression | $0.30 / ₹25 |
| **Resistors** | 100 Ω (4x) & 10 kΩ (4x) 1/4W | 8 | Gate damping & pull-down resistors | $0.20 / ₹15 |
| **Voltage Divider** | 100 kΩ 1% Metal Film | 2 | LiPo battery telemetry monitoring | $0.10 / ₹10 |
| **Step-Down Regulator** | Mini DC-DC Buck / AMS1117-3.3V | 1 | Clean 3.3V power supply for ESP32 and MPU6050 | $1.20 / ₹100 |
| **Filter Capacitor** | 100 µF 16V Low-ESR Electrolytic | 2 | Motor bus decoupling & brownout prevention | $0.30 / ₹25 |

---

## 2. Motors, Propellers & Frame

| Component | Specification / Model | Qty | Notes | Est. Cost (USD / INR) |
|:---|:---|:---:|:---|:---|
| **Coreless Motors** | 8520 / 716 Coreless DC Motors (3.7V) | 4 | 2x CW (Clockwise) + 2x CCW (Counter-Clockwise) | $5.00 / ₹420 |
| **Propellers** | 55mm - 65mm Drone Props | 4 | 2x CW + 2x CCW matched sets | $1.00 / ₹80 |
| **Drone Frame** | 90mm - 110mm Carbon Fiber or 3D Printed Quad-X | 1 | Lightweight frame with motor mounts | $3.50 / ₹300 |
| **Battery** | 1S 3.7V 600mAh - 850mAh 25C/30C LiPo | 1 | High discharge rate battery with JST/PH2.0 connector | $4.00 / ₹350 |
| **Battery Charger** | TP4056 1S LiPo USB Charger Module | 1 | Bench recharging | $0.50 / ₹40 |

---

## 3. Alternative: Brushless Quadcopter Build

If building a larger drone (e.g. 5-inch or 3-inch brushless quad):
- **Motors**: 4x 1404 / 2205 Brushless Motors
- **ESCs**: 4x 20A–30A BLHeli_S ESCs (or 4-in-1 ESC Board)
- **Battery**: 2S or 3S LiPo Battery (7.4V - 11.1V, 850mAh–1300mAh)
- **Regulator**: 5V 3A BEC (Battery Elimination Circuit) to power ESP32 Vin
