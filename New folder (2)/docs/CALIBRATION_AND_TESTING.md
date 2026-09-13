# Calibration, Bench Testing & Safety Checklist

Follow these safety procedures before attempting your first flight.

---

## 1. Safety Rules (CRITICAL)

> [!WARNING]
> **REMOVE ALL PROPELLERS** during bench testing, firmware flashing, and sensor calibration. A sudden motor spin-up with propellers attached can cause injury or damage.

---

## 2. Motor Rotation Direction Verification

A standard **Quad-X** drone configuration requires specific rotation directions:

```
    [M1: FL] (CW) ↻             ↺ [M2: FR] (CCW)
             \                /
              \    FRONT     /
               \   [ESP32]  /
              /    BACK      \
             /                \
    [M3: RL] (CCW) ↺            ↻ [M4: RR] (CW)
```

### Rotation Verification Steps:
1. Keep propellers **OFF**.
2. Connect to the Wi-Fi AP (`ESP32-Drone-Control`).
3. Open `http://192.168.4.1` in your browser.
4. Click **DISARMED** to toggle to **ARMED**.
5. Slowly raise the Throttle slider to **10%**.
6. Inspect the rotation of each motor shaft:
   - **M1 (Front Left)**: Must rotate **Clockwise (CW)**.
   - **M2 (Front Right)**: Must rotate **Counter-Clockwise (CCW)**.
   - **M3 (Rear Left)**: Must rotate **Counter-Clockwise (CCW)**.
   - **M4 (Rear Right)**: Must rotate **Clockwise (CW)**.
7. If a coreless motor spins the wrong way, reverse the polarity of its two wires.
8. If a brushless motor spins the wrong way, swap any two of its three ESC wires.

---

## 3. MPU-6050 Orientation & Gyro Test

1. Place the drone on a flat, level surface.
2. In the web dashboard, observe the **Attitude Indicator (Artificial Horizon)**.
3. Tilt the drone nose-down:
   - The artificial horizon should tilt upwards (showing positive pitch angle).
4. Tilt the drone to the right:
   - The artificial horizon should tilt left (banking right).
5. If angles move in reverse, verify that the MPU-6050 module is mounted facing forward with the Z-axis pointing up.

---

## 4. Fail-Safe Verification Checklist

1. With motors spinning at 15% on the bench (NO PROPS):
2. Turn off Wi-Fi on your phone or laptop.
3. **Verify**: Within **1.0 second**, all motors must immediately cut to 0 RPM.
4. Turn Wi-Fi back on and reconnect. The status should read `DISARMED`.
5. This confirms that the software watchdog fail-safe is functioning properly.

---

## 5. First Flight Checklist

- [ ] Propellers inspected for cracks and correctly mounted (CW props on M1 & M4, CCW props on M2 & M3).
- [ ] Battery fully charged (4.20V per cell for 1S).
- [ ] Fly outdoors or in an open area with a soft landing surface (grass or carpet).
- [ ] Start with gentle throttle pulses to verify take-off balance before committing to a hover.
- [ ] Keep finger near the **EMERGENCY KILL** button / Spacebar at all times.
