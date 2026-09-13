/**
 * ============================================================================
 * Web-Based Drone Control System
 * Configuration & Pin Definitions
 * Author: Alan Tomar (Jun 2024)
 * ============================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// 1. OPERATION & MOTOR DRIVE MODE
// ============================================================================
// Choose motor driver type:
// 1 = Coreless Brushed DC Motors driven by N-Channel MOSFETs (500Hz PWM, 0-255)
// 2 = Brushless Motors with ESCs (50Hz - 400Hz Servo Pulses, 1000µs - 2000µs)
#define MOTOR_DRIVE_MODE 1 

// ============================================================================
// 2. WI-FI & NETWORK CONFIGURATION
// ============================================================================
#define WIFI_AP_MODE true          // true: ESP32 broadcasts its own Wi-Fi Hotspot
                                   // false: ESP32 connects to existing router

#define AP_SSID       "ESP32-Drone-Control"
#define AP_PASSWORD   "drone12345"  // WPA2-PSK (minimum 8 characters)
#define AP_CHANNEL    6
#define AP_MAX_CONN   2

// If WIFI_AP_MODE is false (Station Mode):
#define STA_SSID      "Your_Home_WiFi"
#define STA_PASSWORD  "Your_WiFi_Password"

#define WS_PORT       80           // WebSocket runs on standard HTTP port 80

// ============================================================================
// 3. GPIO PIN ALLOCATION (ESP32 DevKit V1)
// ============================================================================
// Motor Quad-X Layout:
//      [FL: M1] (CW)        [FR: M2] (CCW)
//             \            /
//              \   FRONT  /
//               \ [ESP32]/
//              /   BACK   \
//             /            \
//      [RL: M3] (CCW)       [RR: M4] (CW)

#define PIN_MOTOR_FL   25  // Front-Left Motor (GPIO 25)
#define PIN_MOTOR_FR   26  // Front-Right Motor (GPIO 26)
#define PIN_MOTOR_RL   32  // Rear-Left Motor (GPIO 32)
#define PIN_MOTOR_RR   33  // Rear-Right Motor (GPIO 33)

// I2C Pins for MPU-6050 6-DOF IMU
#define PIN_I2C_SDA    21  // Standard ESP32 I2C Data
#define PIN_I2C_SCL    22  // Standard ESP32 I2C Clock

// Analog Battery Voltage Monitor (via voltage divider)
#define PIN_BATTERY_ADC 34  // ADC1 Channel 6 (Input Only, safe)
#define VOLTAGE_DIVIDER_RATIO 2.0f // Adjust according to R1 & R2 resistor values
#define ADC_REF_VOLTAGE       3.3f
#define ADC_RESOLUTION        4095.0f

// Status Indicators
#define PIN_LED_STATUS  2   // Onboard Blue LED

// ============================================================================
// 4. PWM & TIMER PARAMETERS (ESP32 LEDC)
// ============================================================================
#if MOTOR_DRIVE_MODE == 1
  // Brushed Coreless Motors (Direct MOSFET drive)
  #define PWM_FREQ         500      // 500 Hz for high efficiency smooth torque
  #define PWM_RESOLUTION   8        // 8-bit resolution (0 - 255)
  #define PWM_DUTY_MAX     255
  #define THROTTLE_MIN_ARM 20       // Spin idle speed when armed
#else
  // Brushless Motors (Electronic Speed Controllers)
  #define PWM_FREQ         50       // 50 Hz standard servo frequency (20ms period)
  #define PWM_RESOLUTION   16       // 16-bit resolution
  #define PWM_DUTY_MIN     3276     // 1000 µs (1ms pulse = 0% throttle)
  #define PWM_DUTY_MAX     6553     // 2000 µs (2ms pulse = 100% throttle)
  #define THROTTLE_MIN_ARM 3400
#endif

#define PWM_CH_FL       0
#define PWM_CH_FR       1
#define PWM_CH_RL       2
#define PWM_CH_RR       3

// ============================================================================
// 5. SAFETY & FAIL-SAFE PARAMETERS
// ============================================================================
#define FAILSAFE_TIMEOUT_MS  1000   // If no Wi-Fi packet received for 1000ms, DISARM
#define MAX_TILT_ANGLE_DEG   45.0f  // Emergency cutoff if drone tilts > 45 degrees
#define TELEMETRY_RATE_MS    100    // Send telemetry back to browser every 100ms (10Hz)

// ============================================================================
// 6. FLIGHT STABILIZATION PID GAINS (Default tuning)
// ============================================================================
struct PIDConfig {
  float kp;
  float ki;
  float kd;
};

// Initial conservative P-I-D constants
static PIDConfig pid_roll  = { 1.35f, 0.04f, 0.22f };
static PIDConfig pid_pitch = { 1.35f, 0.04f, 0.22f };
static PIDConfig pid_yaw   = { 2.50f, 0.02f, 0.00f };

#endif // CONFIG_H
