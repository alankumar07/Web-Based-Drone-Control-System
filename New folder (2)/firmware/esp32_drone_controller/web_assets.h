#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

#include <Arduino.h>

// Embedded HTML, CSS & JavaScript Ground Control Station (GCS)
// Stored in ESP32 Flash Memory (PROGMEM) for zero-dependency standalone operation
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>ESP32 Drone Ground Control</title>
  <style>
    :root {
      --bg-dark: #0a0d14;
      --panel-bg: rgba(16, 23, 38, 0.75);
      --border-color: rgba(56, 189, 248, 0.2);
      --neon-cyan: #00f2fe;
      --neon-blue: #4facfe;
      --neon-emerald: #10b981;
      --neon-amber: #f59e0b;
      --neon-red: #ef4444;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
    }
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-select: none;
      touch-action: none;
    }
    body {
      background: radial-gradient(circle at center, #111827 0%, #030712 100%);
      color: var(--text-main);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      overflow: hidden;
    }
    header {
      background: var(--panel-bg);
      backdrop-filter: blur(12px);
      border-bottom: 1px solid var(--border-color);
      padding: 10px 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      z-index: 10;
    }
    .brand {
      display: flex;
      align-items: center;
      gap: 10px;
      font-size: 1.1rem;
      font-weight: 700;
      letter-spacing: 1px;
      color: var(--neon-cyan);
    }
    .badge {
      display: inline-block;
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--neon-red);
      box-shadow: 0 0 8px var(--neon-red);
      transition: all 0.3s ease;
    }
    .badge.connected {
      background: var(--neon-emerald);
      box-shadow: 0 0 10px var(--neon-emerald);
    }
    .telemetry-bar {
      display: flex;
      gap: 20px;
      font-size: 0.85rem;
      color: var(--text-muted);
    }
    .telemetry-item span {
      font-weight: bold;
      color: var(--text-main);
    }
    .btn-arm {
      background: #374151;
      color: white;
      border: 1px solid rgba(255,255,255,0.1);
      padding: 6px 14px;
      border-radius: 6px;
      font-weight: 700;
      cursor: pointer;
      letter-spacing: 0.5px;
      transition: all 0.2s;
    }
    .btn-arm.armed {
      background: var(--neon-emerald);
      color: #064e3b;
      box-shadow: 0 0 12px rgba(16, 185, 129, 0.5);
    }
    .btn-kill {
      background: var(--neon-red);
      color: white;
      border: none;
      padding: 6px 16px;
      border-radius: 6px;
      font-weight: 800;
      cursor: pointer;
      box-shadow: 0 0 12px rgba(239, 68, 68, 0.4);
    }
    .btn-kill:active { transform: scale(0.95); }

    main {
      flex: 1;
      display: grid;
      grid-template-columns: 1fr 1fr 1fr;
      padding: 15px;
      gap: 15px;
      position: relative;
    }
    @media (max-width: 850px) {
      main {
        grid-template-columns: 1fr;
        grid-template-rows: 200px 1fr;
      }
      .center-panel { order: -1; }
    }
    .card {
      background: var(--panel-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--border-color);
      border-radius: 12px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      position: relative;
      padding: 10px;
    }
    .card-title {
      position: absolute;
      top: 10px;
      font-size: 0.75rem;
      text-transform: uppercase;
      letter-spacing: 1.5px;
      color: var(--neon-blue);
    }
    .joystick-container {
      width: 220px;
      height: 220px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(30, 41, 59, 0.6) 0%, rgba(15, 23, 42, 0.9) 100%);
      border: 2px solid rgba(56, 189, 248, 0.3);
      box-shadow: inset 0 0 20px rgba(0, 242, 254, 0.1), 0 0 15px rgba(0,0,0,0.5);
      position: relative;
      touch-action: none;
    }
    .joystick-handle {
      width: 70px;
      height: 70px;
      border-radius: 50%;
      background: linear-gradient(135deg, #0284c7, #0369a1);
      border: 2px solid var(--neon-cyan);
      box-shadow: 0 0 15px rgba(0, 242, 254, 0.5);
      position: absolute;
      top: calc(50% - 35px);
      left: calc(50% - 35px);
      pointer-events: none;
      transform: translate(0, 0);
      transition: transform 0.05s ease-out;
    }
    .center-panel {
      display: flex;
      flex-direction: column;
      gap: 15px;
      justify-content: space-between;
    }
    #horizon-canvas {
      width: 170px;
      height: 170px;
      border-radius: 50%;
      border: 3px solid rgba(56, 189, 248, 0.4);
      box-shadow: 0 0 20px rgba(0,0,0,0.7);
      background: #111;
    }
    .motor-meters {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      width: 100%;
    }
    .motor-meter {
      background: rgba(15, 23, 42, 0.6);
      padding: 6px 10px;
      border-radius: 6px;
      border: 1px solid rgba(255,255,255,0.05);
      font-size: 0.75rem;
    }
    .motor-meter-bar {
      height: 6px;
      background: #334155;
      border-radius: 3px;
      margin-top: 4px;
      overflow: hidden;
    }
    .motor-meter-fill {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, var(--neon-cyan), var(--neon-emerald));
      transition: width 0.1s linear;
    }
    footer {
      background: rgba(3, 7, 18, 0.8);
      border-top: 1px solid rgba(255,255,255,0.05);
      padding: 8px 20px;
      display: flex;
      justify-content: space-between;
      font-size: 0.75rem;
      color: var(--text-muted);
    }
  </style>
</head>
<body>
  <header>
    <div class="brand">
      <div id="status-badge" class="badge"></div>
      <span>AERO-LINK // GCS</span>
    </div>
    <div class="telemetry-bar">
      <div class="telemetry-item">BATT: <span id="val-batt">-- V</span></div>
      <div class="telemetry-item">RSSI: <span id="val-rssi">-- dBm</span></div>
      <div class="telemetry-item">PING: <span id="val-ping">-- ms</span></div>
    </div>
    <div style="display: flex; gap: 10px;">
      <button id="btn-arm" class="btn-arm" onclick="toggleArm()">DISARMED</button>
      <button class="btn-kill" onclick="emergencyStop()">KILL</button>
    </div>
  </header>

  <main>
    <!-- Left Stick: Throttle & Yaw -->
    <div class="card">
      <div class="card-title">Throttle (Z) / Yaw (X)</div>
      <div id="joy-left" class="joystick-container">
        <div id="handle-left" class="joystick-handle"></div>
      </div>
    </div>

    <!-- Center: Attitude Horizon & Motor Gauges -->
    <div class="card center-panel">
      <div class="card-title">Attitude Indicator</div>
      <div style="margin-top: 20px;">
        <canvas id="horizon-canvas" width="200" height="200"></canvas>
      </div>
      <div class="motor-meters">
        <div class="motor-meter">
          <div>FL (M1): <span id="val-m1">0%</span></div>
          <div class="motor-meter-bar"><div id="bar-m1" class="motor-meter-fill"></div></div>
        </div>
        <div class="motor-meter">
          <div>FR (M2): <span id="val-m2">0%</span></div>
          <div class="motor-meter-bar"><div id="bar-m2" class="motor-meter-fill"></div></div>
        </div>
        <div class="motor-meter">
          <div>RL (M3): <span id="val-m3">0%</span></div>
          <div class="motor-meter-bar"><div id="bar-m3" class="motor-meter-fill"></div></div>
        </div>
        <div class="motor-meter">
          <div>RR (M4): <span id="val-m4">0%</span></div>
          <div class="motor-meter-bar"><div id="bar-m4" class="motor-meter-fill"></div></div>
        </div>
      </div>
    </div>

    <!-- Right Stick: Pitch & Roll -->
    <div class="card">
      <div class="card-title">Pitch (Y) / Roll (X)</div>
      <div id="joy-right" class="joystick-container">
        <div id="handle-right" class="joystick-handle"></div>
      </div>
    </div>
  </main>

  <footer>
    <div>Flight Controller: ESP32 Rev1 | Quad-X Configuration</div>
    <div>Telemetry Sync: 10 Hz | Control Rate: 50 Hz</div>
  </footer>

  <script>
    // Flight Control State
    const state = {
      connected: false,
      armed: false,
      throttle: 0,
      yaw: 0,
      pitch: 0,
      roll: 0,
      pingStart: 0,
      latency: 0
    };

    // DOM References
    const statusBadge = document.getElementById('status-badge');
    const btnArm = document.getElementById('btn-arm');
    const valBatt = document.getElementById('val-batt');
    const valRssi = document.getElementById('val-rssi');
    const valPing = document.getElementById('val-ping');
    const horizonCanvas = document.getElementById('horizon-canvas');
    const horizonCtx = horizonCanvas.getContext('2d');

    // WebSocket Connection
    let ws = null;
    function initWebSocket() {
      const wsUri = `ws://${window.location.hostname || '192.168.4.1'}/ws`;
      ws = new WebSocket(wsUri);

      ws.onopen = () => {
        state.connected = true;
        statusBadge.classList.add('connected');
        console.log("WebSocket connected to Drone");
      };

      ws.onclose = () => {
        state.connected = false;
        state.armed = false;
        updateArmUI();
        statusBadge.classList.remove('connected');
        setTimeout(initWebSocket, 2000);
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          if (data.type === 'telemetry') {
            valBatt.textContent = `${data.batt.toFixed(1)}V`;
            valRssi.textContent = `${data.rssi} dBm`;
            state.armed = data.armed;
            updateArmUI();

            // Motors
            updateMotor('m1', data.m1);
            updateMotor('m2', data.m2);
            updateMotor('m3', data.m3);
            updateMotor('m4', data.m4);

            // Horizon render
            renderHorizon(data.pitch || 0, data.roll || 0);

            // Ping calc
            if (data.echoTime) {
              const rtt = Date.now() - data.echoTime;
              valPing.textContent = `${rtt} ms`;
            }
          }
        } catch (e) {}
      };
    }

    function updateMotor(id, val) {
      document.getElementById(`val-${id}`).textContent = `${Math.round(val)}%`;
      document.getElementById(`bar-${id}`).style.width = `${Math.min(100, Math.max(0, val))}%`;
    }

    function toggleArm() {
      if (!state.connected) return;
      if (state.throttle > 5) {
        alert("Safety Warning: Drop throttle to 0% before arming!");
        return;
      }
      state.armed = !state.armed;
      sendPacket({ type: "command", action: state.armed ? "ARM" : "DISARM" });
      updateArmUI();
    }

    function updateArmUI() {
      if (state.armed) {
        btnArm.textContent = "ARMED";
        btnArm.classList.add('armed');
      } else {
        btnArm.textContent = "DISARMED";
        btnArm.classList.remove('armed');
      }
    }

    function emergencyStop() {
      state.armed = false;
      state.throttle = 0;
      updateArmUI();
      sendPacket({ type: "command", action: "KILL" });
    }

    function sendPacket(data) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(data));
      }
    }

    // Send Control Input Loop (50 Hz)
    setInterval(() => {
      if (state.connected) {
        sendPacket({
          type: "stick",
          t: Math.round(state.throttle),
          y: Math.round(state.yaw),
          p: Math.round(state.pitch),
          r: Math.round(state.roll),
          ts: Date.now()
        });
      }
    }, 20);

    // Virtual Touch Joysticks
    function setupJoystick(containerId, handleId, isThrottleYaw) {
      const container = document.getElementById(containerId);
      const handle = document.getElementById(handleId);
      const maxDist = 70;
      let active = false;
      let startX = 0, startY = 0;

      function updatePosition(x, y) {
        let dx = x - startX;
        let dy = y - startY;
        const dist = Math.hypot(dx, dy);

        if (dist > maxDist) {
          dx = (dx / dist) * maxDist;
          dy = (dy / dist) * maxDist;
        }

        handle.style.transform = `translate(${dx}px, ${dy}px)`;

        const normX = (dx / maxDist);
        const normY = -(dy / maxDist);

        if (isThrottleYaw) {
          state.yaw = normX * 45; // -45 to +45 deg/s
          // Throttle ranges 0 to 100
          state.throttle = Math.max(0, Math.min(100, (normY + 1) * 50));
        } else {
          state.roll = normX * 30; // -30 to +30 deg
          state.pitch = normY * 30; // -30 to +30 deg
        }
      }

      function handleStart(e) {
        active = true;
        const rect = container.getBoundingClientRect();
        startX = rect.left + rect.width / 2;
        startY = rect.top + rect.height / 2;
        const pt = e.touches ? e.touches[0] : e;
        updatePosition(pt.clientX, pt.clientY);
      }

      function handleMove(e) {
        if (!active) return;
        const pt = e.touches ? e.touches[0] : e;
        updatePosition(pt.clientX, pt.clientY);
      }

      function handleEnd() {
        active = false;
        if (!isThrottleYaw) {
          // Right stick springs back to center
          handle.style.transform = `translate(0px, 0px)`;
          state.pitch = 0;
          state.roll = 0;
        } else {
          // Left stick yaw springs back to 0, throttle stays
          const curY = -((state.throttle / 50) - 1) * maxDist;
          handle.style.transform = `translate(0px, ${curY}px)`;
          state.yaw = 0;
        }
      }

      container.addEventListener('pointerdown', handleStart);
      window.addEventListener('pointermove', handleMove);
      window.addEventListener('pointerup', handleEnd);
      window.addEventListener('pointercancel', handleEnd);
    }

    setupJoystick('joy-left', 'handle-left', true);
    setupJoystick('joy-right', 'handle-right', false);

    // Artificial Horizon Canvas Render
    function renderHorizon(pitchDeg, rollDeg) {
      const w = horizonCanvas.width;
      const h = horizonCanvas.height;
      const cx = w / 2;
      const cy = h / 2;
      const rad = (rollDeg * Math.PI) / 180;
      const pitchOffset = (pitchDeg / 90) * (h / 2);

      horizonCtx.clearRect(0, 0, w, h);
      horizonCtx.save();

      // Clip to circular gauge
      horizonCtx.beginPath();
      horizonCtx.arc(cx, cy, cx - 4, 0, Math.PI * 2);
      horizonCtx.clip();

      // Rotate for Roll
      horizonCtx.translate(cx, cy);
      horizonCtx.rotate(-rad);
      horizonCtx.translate(-cx, -cy);

      // Sky
      horizonCtx.fillStyle = '#0284c7';
      horizonCtx.fillRect(-w, -h + pitchOffset, w * 3, h);

      // Ground
      horizonCtx.fillStyle = '#854d0e';
      horizonCtx.fillRect(-w, cy + pitchOffset, w * 3, h * 2);

      // Horizon line
      horizonCtx.strokeStyle = '#ffffff';
      horizonCtx.lineWidth = 2;
      horizonCtx.beginPath();
      horizonCtx.moveTo(-w, cy + pitchOffset);
      horizonCtx.lineTo(w * 2, cy + pitchOffset);
      horizonCtx.stroke();

      horizonCtx.restore();

      // Stationary Reticle / Crosshair
      horizonCtx.strokeStyle = '#fbbf24';
      horizonCtx.lineWidth = 3;
      horizonCtx.beginPath();
      horizonCtx.moveTo(cx - 30, cy);
      horizonCtx.lineTo(cx - 10, cy);
      horizonCtx.lineTo(cx, cy + 8);
      horizonCtx.lineTo(cx + 10, cy);
      horizonCtx.lineTo(cx + 30, cy);
      horizonCtx.stroke();
    }

    // Initialize
    window.addEventListener('DOMContentLoaded', () => {
      renderHorizon(0, 0);
      initWebSocket();
    });
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_ASSETS_H
