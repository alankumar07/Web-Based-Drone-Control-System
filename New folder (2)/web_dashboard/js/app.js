/**
 * Main Ground Control Station Application Logic
 * Integrates WebSockets, Gamepad API, Keyboard Inputs, and HUD Gauges
 * Author: Alan Tomar (Jun 2024)
 */

document.addEventListener('DOMContentLoaded', () => {
  // Application State
  const flightState = {
    connected: false,
    armed: false,
    targetThrottle: 0, // 0 to 100 %
    targetYaw: 0,      // -45 to +45 deg/s
    targetPitch: 0,    // -30 to +30 deg
    targetRoll: 0,     // -30 to +30 deg
    droneIP: localStorage.getItem('esp32_drone_ip') || '192.168.4.1',
    lastPingSent: 0,
    rtt: 0,
    isDemoMode: false
  };

  // DOM Elements
  const statusDot = document.getElementById('status-dot');
  const connStatusText = document.getElementById('conn-status-text');
  const btnDemo = document.getElementById('btn-demo');
  const btnArm = document.getElementById('btn-arm');
  const btnKill = document.getElementById('btn-kill');
  const btnSettings = document.getElementById('btn-settings');
  const settingsModal = document.getElementById('settings-modal');
  const btnSaveIp = document.getElementById('btn-save-ip');
  const inputDroneIp = document.getElementById('input-drone-ip');
  const consoleLog = document.getElementById('console-log');
  
  const stickYawVal = document.getElementById('stick-yaw-val');
  const stickThrottleVal = document.getElementById('stick-throttle-val');
  const stickRollVal = document.getElementById('stick-roll-val');
  const stickPitchVal = document.getElementById('stick-pitch-val');

  inputDroneIp.value = flightState.droneIP;

  // Initialize Telemetry Visualizer
  const telemetry = new TelemetryDisplay('attitudeCanvas');

  // Logging utility
  function log(msg, type = 'info') {
    const entry = document.createElement('div');
    entry.className = `log-entry ${type}`;
    const time = new Date().toLocaleTimeString();
    entry.textContent = `[${time}] ${msg}`;
    consoleLog.appendChild(entry);
    consoleLog.scrollTop = consoleLog.scrollHeight;
  }

  // Initialize Joysticks
  const leftJoystick = new VirtualJoystick({
    zoneId: 'joy-zone-left',
    puckId: 'joy-puck-left',
    isThrottleYaw: true,
    onChange: (coords) => {
      flightState.targetYaw = coords.x * 45; // -45 to +45 deg/s
      flightState.targetThrottle = coords.y; // 0 to 100%
      stickYawVal.textContent = `${Math.round(flightState.targetYaw)}°/s`;
      stickThrottleVal.textContent = `${Math.round(flightState.targetThrottle)}%`;
    }
  });

  const rightJoystick = new VirtualJoystick({
    zoneId: 'joy-zone-right',
    puckId: 'joy-puck-right',
    isThrottleYaw: false,
    onChange: (coords) => {
      flightState.targetRoll = coords.x * 30; // -30 to +30 deg
      flightState.targetPitch = coords.y * 30; // -30 to +30 deg
      stickRollVal.textContent = `${Math.round(flightState.targetRoll)}°`;
      stickPitchVal.textContent = `${Math.round(flightState.targetPitch)}°`;
    }
  });

  // WebSocket Client
  let ws = null;
  let reconnectTimer = null;

  function connectWebSocket() {
    if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
      return;
    }

    const wsUrl = `ws://${flightState.droneIP}/ws`;
    log(`Connecting to flight controller at ${wsUrl}...`, 'info');

    try {
      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        flightState.connected = true;
        statusDot.classList.add('connected');
        connStatusText.textContent = `ONLINE (${flightState.droneIP})`;
        log('Telemetry link established! 50Hz control loop active.', 'info');
      };

      ws.onclose = () => {
        flightState.connected = false;
        flightState.armed = false;
        updateArmStateUI();
        statusDot.classList.remove('connected');
        connStatusText.textContent = 'DISCONNECTED';
        log('Connection lost. Reconnecting in 3s...', 'warn');
        clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(connectWebSocket, 3000);
      };

      ws.onerror = (err) => {
        log('WebSocket link error.', 'danger');
      };

      ws.onmessage = (event) => {
        try {
          const packet = JSON.parse(event.data);
          if (packet.type === 'telemetry') {
            telemetry.updateAttitude(packet.pitch || 0, packet.roll || 0, packet.yaw || 0);
            telemetry.updateMotors(packet.m1 || 0, packet.m2 || 0, packet.m3 || 0, packet.m4 || 0);
            telemetry.updateBattery(packet.batt || 0);
            telemetry.updateRSSI(packet.rssi || 0);

            if (packet.echoTime && flightState.lastPingSent) {
              const rtt = Date.now() - packet.echoTime;
              telemetry.updatePing(rtt);
            }

            if (flightState.armed !== packet.armed) {
              flightState.armed = packet.armed;
              updateArmStateUI();
            }
          }
        } catch (e) {
          // Packet parse error
        }
      };
    } catch (err) {
      log(`Connection setup error: ${err.message}`, 'danger');
    }
  }

  function sendPacket(data) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify(data));
    }
  }

  // 50 Hz Control Command Dispatch Loop (20ms interval)
  setInterval(() => {
    if (flightState.connected) {
      flightState.lastPingSent = Date.now();
      sendPacket({
        type: 'stick',
        t: Math.round(flightState.targetThrottle),
        y: Math.round(flightState.targetYaw),
        p: Math.round(flightState.targetPitch),
        r: Math.round(flightState.targetRoll),
        ts: flightState.lastPingSent
      });
    }

    // In Demo / Simulation Mode: update attitudes and motors locally
    if (flightState.isDemoMode) {
      telemetry.updateAttitude(flightState.targetPitch, flightState.targetRoll, flightState.targetYaw);
      if (flightState.armed) {
        const t = flightState.targetThrottle;
        const p = flightState.targetPitch;
        const r = flightState.targetRoll;
        const y = flightState.targetYaw;
        const m1 = Math.max(0, Math.min(100, t + p + r - y));
        const m2 = Math.max(0, Math.min(100, t + p - r + y));
        const m3 = Math.max(0, Math.min(100, t - p + r + y));
        const m4 = Math.max(0, Math.min(100, t - p - r - y));
        telemetry.updateMotors(m1, m2, m3, m4);
      } else {
        telemetry.updateMotors(0, 0, 0, 0);
      }
    }
  }, 20);

  // Demo Flight Mode Handler
  btnDemo.addEventListener('click', () => {
    flightState.isDemoMode = !flightState.isDemoMode;
    if (flightState.isDemoMode) {
      btnDemo.style.background = 'var(--neon-emerald)';
      btnDemo.style.color = '#022c22';
      statusDot.classList.add('connected');
      connStatusText.textContent = 'SIMULATION (DEMO)';
      telemetry.updateBattery(4.15);
      telemetry.updateRSSI(-42);
      telemetry.updatePing(8);
      log('Simulation / Demo Flight Mode ENABLED! Try Joysticks, Keys (W/S/A/D), or Gamepad.', 'info');
    } else {
      btnDemo.style.background = 'rgba(16, 185, 129, 0.15)';
      btnDemo.style.color = 'var(--neon-emerald)';
      if (!flightState.connected) {
        statusDot.classList.remove('connected');
        connStatusText.textContent = 'DISCONNECTED';
      }
      flightState.armed = false;
      updateArmStateUI();
      telemetry.updateMotors(0, 0, 0, 0);
      log('Simulation / Demo Flight Mode DISABLED.', 'warn');
    }
  });

  // Arming and Kill Control Handlers
  function updateArmStateUI() {
    if (flightState.armed) {
      btnArm.textContent = 'ARMED (ACTIVE)';
      btnArm.classList.add('armed');
    } else {
      btnArm.textContent = 'DISARMED';
      btnArm.classList.remove('armed');
    }
  }

  btnArm.addEventListener('click', () => {
    if (!flightState.connected && !flightState.isDemoMode) {
      log('Cannot arm: Drone not connected! (Tip: Click DEMO FLIGHT to test)', 'warn');
      return;
    }
    if (!flightState.armed && flightState.targetThrottle > 5) {
      alert('Safety Guard: Throttle must be at 0% to ARM!');
      return;
    }

    const nextArm = !flightState.armed;
    if (flightState.connected) {
      sendPacket({ type: 'command', action: nextArm ? 'ARM' : 'DISARM' });
    }
    flightState.armed = nextArm;
    updateArmStateUI();
    log(`Drone ${nextArm ? 'ARMED' : 'DISARMED'} by operator.`, nextArm ? 'warn' : 'info');
  });

  btnKill.addEventListener('click', () => {
    flightState.armed = false;
    flightState.targetThrottle = 0;
    leftJoystick.setThrottleDirect(0);
    updateArmStateUI();
    if (flightState.connected) {
      sendPacket({ type: 'command', action: 'KILL' });
    }
    telemetry.updateMotors(0, 0, 0, 0);
    log('EMERGENCY KILL SWITCH TRIGGERED!', 'danger');
  });

  // Settings Modal Handlers
  btnSettings.addEventListener('click', () => {
    settingsModal.style.display = 'flex';
  });

  btnSaveIp.addEventListener('click', () => {
    const val = inputDroneIp.value.trim();
    if (val) {
      flightState.droneIP = val;
      localStorage.setItem('esp32_drone_ip', val);
      settingsModal.style.display = 'none';
      if (ws) ws.close();
      connectWebSocket();
    }
  });

  settingsModal.addEventListener('click', (e) => {
    if (e.target === settingsModal) settingsModal.style.display = 'none';
  });

  // Keyboard Shortcuts (W/S Throttle, A/D Yaw, Arrow keys Pitch/Roll, Space Kill)
  window.addEventListener('keydown', (e) => {
    if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', ' '].includes(e.key)) {
      e.preventDefault();
    }

    switch (e.key.toLowerCase()) {
      case 'w':
        flightState.targetThrottle = Math.min(100, flightState.targetThrottle + 3);
        leftJoystick.setThrottleDirect(flightState.targetThrottle);
        break;
      case 's':
        flightState.targetThrottle = Math.max(0, flightState.targetThrottle - 3);
        leftJoystick.setThrottleDirect(flightState.targetThrottle);
        break;
      case 'a':
        flightState.targetYaw = -30;
        break;
      case 'd':
        flightState.targetYaw = 30;
        break;
      case 'arrowup':
        flightState.targetPitch = 20;
        break;
      case 'arrowdown':
        flightState.targetPitch = -20;
        break;
      case 'arrowleft':
        flightState.targetRoll = -20;
        break;
      case 'arrowright':
        flightState.targetRoll = 20;
        break;
      case ' ':
        btnKill.click();
        break;
    }
  });

  window.addEventListener('keyup', (e) => {
    switch (e.key.toLowerCase()) {
      case 'a':
      case 'd':
        flightState.targetYaw = 0;
        break;
      case 'arrowup':
      case 'arrowdown':
        flightState.targetPitch = 0;
        break;
      case 'arrowleft':
      case 'arrowright':
        flightState.targetRoll = 0;
        break;
    }
  });

  // Gamepad API Integration
  function scanGamepads() {
    const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
    const gp = gamepads[0]; // Primary controller

    if (gp) {
      // Left Stick (Axes 0: Yaw, 1: Throttle)
      const yawRaw = gp.axes[0];
      const throttleRaw = -gp.axes[1]; // Invert Y
      if (Math.abs(yawRaw) > 0.08) flightState.targetYaw = yawRaw * 45;
      else flightState.targetYaw = 0;

      const normThrot = Math.max(0, Math.min(100, (throttleRaw + 1) * 50));
      flightState.targetThrottle = normThrot;
      leftJoystick.setThrottleDirect(normThrot);

      // Right Stick (Axes 2: Roll, 3: Pitch)
      const rollRaw = gp.axes[2];
      const pitchRaw = -gp.axes[3];
      flightState.targetRoll = Math.abs(rollRaw) > 0.08 ? rollRaw * 30 : 0;
      flightState.targetPitch = Math.abs(pitchRaw) > 0.08 ? pitchRaw * 30 : 0;

      // Button 0 (A / Cross): Toggle Arm
      if (gp.buttons[0].pressed && !gp._lastA) {
        btnArm.click();
      }
      gp._lastA = gp.buttons[0].pressed;

      // Button 1 (B / Circle): Emergency Kill
      if (gp.buttons[1].pressed) {
        btnKill.click();
      }
    }

    requestAnimationFrame(scanGamepads);
  }
  requestAnimationFrame(scanGamepads);

  // Initialize
  log('Ground Control Station initialized.', 'info');
  connectWebSocket();
});
