/**
 * ============================================================================
 * Web-Based Drone Control System
 * Main Flight Controller Firmware for ESP32
 * 
 * Author: Alan Tomar (Jun 2024)
 * GitHub: https://github.com/alantomar/Web-Based-Drone-Control-System
 * 
 * Hardware:
 *   - Microcontroller: ESP32 DevKit V1 (30-pin / 38-pin)
 *   - IMU Sensor: MPU-6050 (I2C: SDA=21, SCL=22)
 *   - Actuators: 4x Coreless Brushed Motors (MOSFET) or Brushless Motors (ESC)
 *   - Power: 1S / 2S LiPo with voltage divider on GPIO 34
 * 
 * Libraries Required (Install via Arduino Library Manager):
 *   1. ESPAsyncWebServer (by me-no-dev / lacamera)
 *   2. AsyncTCP (by me-no-dev / dvarrel)
 *   3. Adafruit MPU6050 & Adafruit Sensor
 *   4. ArduinoJson (v6 or v7)
 * ============================================================================
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include "config.h"
#include "web_assets.h"

// ============================================================================
// SYSTEM OBJECTS & GLOBAL VARIABLES
// ============================================================================
AsyncWebServer server(WS_PORT);
AsyncWebSocket ws("/ws");
Adafruit_MPU6050 mpu;

// Flight States
enum FlightState {
  STATE_DISARMED = 0,
  STATE_ARMED    = 1,
  STATE_FAILSAFE = 2
};

volatile FlightState currentState = STATE_DISARMED;

// Control Targets (Received from Web Dashboard)
float targetThrottle = 0.0f; // 0.0 to 100.0 %
float targetYaw      = 0.0f; // -45.0 to +45.0 deg/s
float targetPitch    = 0.0f; // -30.0 to +30.0 deg
float targetRoll     = 0.0f; // -30.0 to +30.0 deg
uint32_t lastEchoTs  = 0;

// Measured Sensor Telemetry
float currentPitch = 0.0f;
float currentRoll  = 0.0f;
float currentYaw   = 0.0f;
float batteryVoltage = 0.0f;
bool mpuAvailable   = false;

// Motor Outputs (0.0 to 100.0 %)
float outputM1_FL = 0.0f;
float outputM2_FR = 0.0f;
float outputM3_RL = 0.0f;
float outputM4_RR = 0.0f;

// Loop Timers
unsigned long lastPacketTime   = 0;
unsigned long lastTelemetryTime = 0;
unsigned long lastPidTime       = 0;

// PID Variables
float errPitchPrev = 0, intPitch = 0;
float errRollPrev  = 0, intRoll  = 0;
float errYawPrev   = 0, intYaw   = 0;

// Function Prototypes
void setupHardware();
void setupWiFi();
void setupWebServer();
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                      AwsEventType type, void *arg, uint8_t *data, size_t len);
void handleCommand(const char* action);
void updateIMU(float dt);
void computePID(float dt);
void applyMotorOutputs();
void disarmMotors();
void sendTelemetry();
float readBattery();

// ============================================================================
// ARDUINO SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n===========================================");
  Serial.println("  Web-Based Drone Control System (ESP32)");
  Serial.println("  Firmware Initializing...");
  Serial.println("===========================================");

  setupHardware();
  setupWiFi();
  setupWebServer();

  lastPacketTime = millis();
  lastPidTime = micros();

  Serial.println("[SYSTEM] Ready! Connect to Wi-Fi and open GCS in browser.");
}

// ============================================================================
// ARDUINO MAIN LOOP
// ============================================================================
void loop() {
  unsigned long nowMs = millis();
  unsigned long nowMicros = micros();
  float dt = (nowMicros - lastPidTime) / 1000000.0f;

  // Run flight stabilization loop at ~250 Hz (every 4000 µs)
  if (dt >= 0.004f) {
    lastPidTime = nowMicros;

    // 1. Read IMU sensor orientation
    if (mpuAvailable) {
      updateIMU(dt);
    }

    // 2. Safety Watchdog Check
    if (currentState == STATE_ARMED) {
      if (nowMs - lastPacketTime > FAILSAFE_TIMEOUT_MS) {
        Serial.println("[SAFETY] Failsafe Triggered: Signal Loss! Disarming.");
        currentState = STATE_FAILSAFE;
        disarmMotors();
      }

      // Emergency tilt angle cutoff (> 45 degrees)
      if (abs(currentPitch) > MAX_TILT_ANGLE_DEG || abs(currentRoll) > MAX_TILT_ANGLE_DEG) {
        Serial.println("[SAFETY] Emergency Cutoff: Excessive Tilt Angle!");
        currentState = STATE_FAILSAFE;
        disarmMotors();
      }
    }

    // 3. Compute PID and update PWM outputs if armed
    if (currentState == STATE_ARMED) {
      computePID(dt);
      applyMotorOutputs();
    } else {
      disarmMotors();
    }
  }

  // Broadcast Telemetry to Browser Ground Control at 10 Hz
  if (nowMs - lastTelemetryTime >= TELEMETRY_RATE_MS) {
    lastTelemetryTime = nowMs;
    batteryVoltage = readBattery();
    sendTelemetry();
    ws.cleanupClients();
  }
}

// ============================================================================
// HARDWARE INITIALIZATION
// ============================================================================
void setupHardware() {
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW);

  // Configure PWM Channels (LEDC)
  ledcSetup(PWM_CH_FL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_FR, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_RL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_RR, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(PIN_MOTOR_FL, PWM_CH_FL);
  ledcAttachPin(PIN_MOTOR_FR, PWM_CH_FR);
  ledcAttachPin(PIN_MOTOR_RL, PWM_CH_RL);
  ledcAttachPin(PIN_MOTOR_RR, PWM_CH_RR);

  disarmMotors();

  // Initialize I2C and MPU6050
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (mpu.begin()) {
    mpuAvailable = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("[IMU] MPU-6050 connected & configured successfully.");
  } else {
    mpuAvailable = false;
    Serial.println("[WARNING] MPU-6050 not found! Bench test mode active.");
  }

  // ADC setup
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

// ============================================================================
// WI-FI INITIALIZATION
// ============================================================================
void setupWiFi() {
#if WIFI_AP_MODE
  WiFi.mode(WIFI_AP);
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONN);

  Serial.println("[WIFI] Access Point Created!");
  Serial.print("  SSID: "); Serial.println(AP_SSID);
  Serial.print("  IP Address: "); Serial.println(WiFi.softAPIP());
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(STA_SSID, STA_PASSWORD);
  Serial.print("[WIFI] Connecting to router");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[WIFI] Connected! Drone IP: ");
  Serial.println(WiFi.localIP());
#endif
}

// ============================================================================
// WEBSERVER & WEBSOCKET SETUP
// ============================================================================
void setupWebServer() {
  // Serve web cockpit directly from ESP32 Flash
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", INDEX_HTML);
  });

  // Attach WebSocket handler
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.begin();
  Serial.println("[HTTP] AsyncWebServer & WebSocket listener running.");
}

// ============================================================================
// WEBSOCKET PACKET PROCESSING
// ============================================================================
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("[WS] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      digitalWrite(PIN_LED_STATUS, HIGH);
      lastPacketTime = millis();
      break;

    case WS_EVT_DISCONNECT:
      Serial.printf("[WS] Client #%u disconnected\n", client->id());
      digitalWrite(PIN_LED_STATUS, LOW);
      if (currentState == STATE_ARMED) {
        currentState = STATE_FAILSAFE;
        disarmMotors();
      }
      break;

    case WS_EVT_DATA: {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        lastPacketTime = millis(); // Refresh watchdog heartbeat

        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (!err) {
          const char* pType = doc["type"];
          if (pType && strcmp(pType, "stick") == 0) {
            targetThrottle = doc["t"] | 0.0f;
            targetYaw      = doc["y"] | 0.0f;
            targetPitch    = doc["p"] | 0.0f;
            targetRoll     = doc["r"] | 0.0f;
            lastEchoTs     = doc["ts"] | 0;
          } else if (pType && strcmp(pType, "command") == 0) {
            const char* action = doc["action"];
            if (action) handleCommand(action);
          }
        }
      }
      break;
    }
    default:
      break;
  }
}

void handleCommand(const char* action) {
  if (strcmp(action, "ARM") == 0) {
    if (targetThrottle <= 5.0f) {
      currentState = STATE_ARMED;
      intPitch = intRoll = intYaw = 0; // Reset integrator
      Serial.println("[COMMAND] Drone ARMED!");
    } else {
      Serial.println("[COMMAND] ARM Rejected: Throttle must be 0!");
    }
  } else if (strcmp(action, "DISARM") == 0) {
    currentState = STATE_DISARMED;
    disarmMotors();
    Serial.println("[COMMAND] Drone DISARMED.");
  } else if (strcmp(action, "KILL") == 0) {
    currentState = STATE_DISARMED;
    targetThrottle = 0;
    disarmMotors();
    Serial.println("[COMMAND] EMERGENCY KILL EXECUTED!");
  }
}

// ============================================================================
// IMU & COMPLEMENTARY FILTER
// ============================================================================
void updateIMU(float dt) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Accelerometer angles
  float accRoll  = atan2(a.acceleration.y, a.acceleration.z) * 180.0f / PI;
  float accPitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0f / PI;

  // Gyro rates in deg/s
  float gyroRollRate  = g.gyro.x * 180.0f / PI;
  float gyroPitchRate = g.gyro.y * 180.0f / PI;
  float gyroYawRate   = g.gyro.z * 180.0f / PI;

  // Complementary filter (96% gyro integration, 4% accelerometer absolute)
  currentRoll  = 0.96f * (currentRoll + gyroRollRate * dt) + 0.04f * accRoll;
  currentPitch = 0.96f * (currentPitch + gyroPitchRate * dt) + 0.04f * accPitch;
  currentYaw  += gyroYawRate * dt;
}

// ============================================================================
// PID CONTROL LOOP
// ============================================================================
float corrPitch = 0.0f;
float corrRoll  = 0.0f;
float corrYaw   = 0.0f;

void computePID(float dt) {
  if (targetThrottle < 5.0f) {
    // Idle spin on ground: don't accumulate integral windup
    intPitch = intRoll = intYaw = 0;
    corrPitch = corrRoll = corrYaw = 0;
    return;
  }

  // Pitch PID
  float errPitch = targetPitch - currentPitch;
  intPitch += errPitch * dt;
  intPitch = constrain(intPitch, -20.0f, 20.0f);
  float dPitch = (errPitch - errPitchPrev) / dt;
  corrPitch = (pid_pitch.kp * errPitch) + (pid_pitch.ki * intPitch) + (pid_pitch.kd * dPitch);
  errPitchPrev = errPitch;

  // Roll PID
  float errRoll = targetRoll - currentRoll;
  intRoll += errRoll * dt;
  intRoll = constrain(intRoll, -20.0f, 20.0f);
  float dRoll = (errRoll - errRollPrev) / dt;
  corrRoll = (pid_roll.kp * errRoll) + (pid_roll.ki * intRoll) + (pid_roll.kd * dRoll);
  errRollPrev = errRoll;

  // Yaw Rate PID
  float errYaw = targetYaw; // Rate control
  corrYaw = pid_yaw.kp * errYaw;
}

// ============================================================================
// MOTOR MIXER & PWM APPLICATION
// ============================================================================
void applyMotorOutputs() {
  if (targetThrottle < 3.0f) {
    // Low idle rotation
    outputM1_FL = THROTTLE_MIN_ARM;
    outputM2_FR = THROTTLE_MIN_ARM;
    outputM3_RL = THROTTLE_MIN_ARM;
    outputM4_RR = THROTTLE_MIN_ARM;
  } else {
    // Quad-X Motor Mixing:
    // M1 Front-Left (CW):   Throttle + Pitch + Roll - Yaw
    // M2 Front-Right (CCW): Throttle + Pitch - Roll + Yaw
    // M3 Rear-Left (CCW):   Throttle - Pitch + Roll + Yaw
    // M4 Rear-Right (CW):   Throttle - Pitch - Roll - Yaw
    float baseThrottle = targetThrottle;

    outputM1_FL = baseThrottle + corrPitch + corrRoll - corrYaw;
    outputM2_FR = baseThrottle + corrPitch - corrRoll + corrYaw;
    outputM3_RL = baseThrottle - corrPitch + corrRoll + corrYaw;
    outputM4_RR = baseThrottle - corrPitch - corrRoll - corrYaw;
  }

  // Clamp percentages between 0% and 100%
  outputM1_FL = constrain(outputM1_FL, 0.0f, 100.0f);
  outputM2_FR = constrain(outputM2_FR, 0.0f, 100.0f);
  outputM3_RL = constrain(outputM3_RL, 0.0f, 100.0f);
  outputM4_RR = constrain(outputM4_RR, 0.0f, 100.0f);

  // Convert percentage to PWM Duty Cycle
#if MOTOR_DRIVE_MODE == 1
  uint32_t dutyM1 = (uint32_t)((outputM1_FL / 100.0f) * PWM_DUTY_MAX);
  uint32_t dutyM2 = (uint32_t)((outputM2_FR / 100.0f) * PWM_DUTY_MAX);
  uint32_t dutyM3 = (uint32_t)((outputM3_RL / 100.0f) * PWM_DUTY_MAX);
  uint32_t dutyM4 = (uint32_t)((outputM4_RR / 100.0f) * PWM_DUTY_MAX);
#else
  uint32_t dutyM1 = map(outputM1_FL, 0, 100, PWM_DUTY_MIN, PWM_DUTY_MAX);
  uint32_t dutyM2 = map(outputM2_FR, 0, 100, PWM_DUTY_MIN, PWM_DUTY_MAX);
  uint32_t dutyM3 = map(outputM3_RL, 0, 100, PWM_DUTY_MIN, PWM_DUTY_MAX);
  uint32_t dutyM4 = map(outputM4_RR, 0, 100, PWM_DUTY_MIN, PWM_DUTY_MAX);
#endif

  ledcWrite(PWM_CH_FL, dutyM1);
  ledcWrite(PWM_CH_FR, dutyM2);
  ledcWrite(PWM_CH_RL, dutyM3);
  ledcWrite(PWM_CH_RR, dutyM4);
}

void disarmMotors() {
  outputM1_FL = outputM2_FR = outputM3_RL = outputM4_RR = 0;
#if MOTOR_DRIVE_MODE == 1
  ledcWrite(PWM_CH_FL, 0);
  ledcWrite(PWM_CH_FR, 0);
  ledcWrite(PWM_CH_RL, 0);
  ledcWrite(PWM_CH_RR, 0);
#else
  ledcWrite(PWM_CH_FL, PWM_DUTY_MIN);
  ledcWrite(PWM_CH_FR, PWM_DUTY_MIN);
  ledcWrite(PWM_CH_RL, PWM_DUTY_MIN);
  ledcWrite(PWM_CH_RR, PWM_DUTY_MIN);
#endif
}

// ============================================================================
// TELEMETRY & BATTERY MONITOR
// ============================================================================
float readBattery() {
  int raw = analogRead(PIN_BATTERY_ADC);
  float measured = (raw / ADC_RESOLUTION) * ADC_REF_VOLTAGE * VOLTAGE_DIVIDER_RATIO;
  return measured;
}

void sendTelemetry() {
  if (ws.count() == 0) return;

  StaticJsonDocument<256> doc;
  doc["type"]     = "telemetry";
  doc["armed"]    = (currentState == STATE_ARMED);
  doc["batt"]     = batteryVoltage;
  doc["rssi"]     = WiFi.RSSI();
  doc["pitch"]    = currentPitch;
  doc["roll"]     = currentRoll;
  doc["yaw"]      = currentYaw;
  doc["m1"]       = outputM1_FL;
  doc["m2"]       = outputM2_FR;
  doc["m3"]       = outputM3_RL;
  doc["m4"]       = outputM4_RR;
  doc["echoTime"] = lastEchoTs;

  char buffer[256];
  size_t len = serializeJson(doc, buffer);
  ws.textAll(buffer, len);
}
