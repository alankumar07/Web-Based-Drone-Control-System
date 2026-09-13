/**
 * Telemetry Display & Artificial Horizon (Attitude Indicator)
 * Author: Alan Tomar (Jun 2024)
 */

class TelemetryDisplay {
  constructor(canvasId) {
    this.canvas = document.getElementById(canvasId);
    this.ctx = this.canvas.getContext('2d');
    this.width = this.canvas.width;
    this.height = this.canvas.height;
    this.cx = this.width / 2;
    this.cy = this.height / 2;
    this.radius = this.cx - 6;

    this.pitch = 0;
    this.roll = 0;
    this.yaw = 0;

    this.renderHorizon(0, 0);
  }

  updateAttitude(pitchDeg, rollDeg, yawDeg) {
    this.pitch = pitchDeg;
    this.roll = rollDeg;
    this.yaw = yawDeg;
    this.renderHorizon(this.pitch, this.roll);
  }

  renderHorizon(pitch, roll) {
    const ctx = this.ctx;
    const w = this.width;
    const h = this.height;
    const cx = this.cx;
    const cy = this.cy;
    const r = this.radius;

    ctx.clearRect(0, 0, w, h);

    ctx.save();
    // Clip round gauge circle
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, Math.PI * 2);
    ctx.clip();

    // Rotate canvas for Roll
    const rollRad = (roll * Math.PI) / 180;
    ctx.translate(cx, cy);
    ctx.rotate(-rollRad);
    ctx.translate(-cx, -cy);

    // Calculate vertical offset for Pitch (pixels per degree)
    const pxPerDeg = 2.0;
    const pitchOffset = pitch * pxPerDeg;

    // 1. Sky (Deep aviation blue)
    ctx.fillStyle = '#0284c7';
    ctx.fillRect(-w, -h * 2 + cy + pitchOffset, w * 3, h * 2);

    // 2. Earth / Ground (Dark earth brown)
    ctx.fillStyle = '#78350f';
    ctx.fillRect(-w, cy + pitchOffset, w * 3, h * 2);

    // 3. Horizon separator line
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 2.5;
    ctx.beginPath();
    ctx.moveTo(-w, cy + pitchOffset);
    ctx.lineTo(w * 2, cy + pitchOffset);
    ctx.stroke();

    // 4. Pitch Ladder (Every 10 degrees up and down)
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.7)';
    ctx.fillStyle = '#ffffff';
    ctx.font = '10px JetBrains Mono, monospace';
    ctx.textAlign = 'center';
    ctx.lineWidth = 1.5;

    for (let deg = -40; deg <= 40; deg += 10) {
      if (deg === 0) continue;
      const yPos = cy + pitchOffset - (deg * pxPerDeg);
      const barWidth = Math.abs(deg) % 20 === 0 ? 36 : 22;

      ctx.beginPath();
      ctx.moveTo(cx - barWidth, yPos);
      ctx.lineTo(cx + barWidth, yPos);
      ctx.stroke();

      if (Math.abs(deg) % 20 === 0) {
        ctx.fillText(`${Math.abs(deg)}°`, cx + barWidth + 14, yPos + 3);
      }
    }

    ctx.restore();

    // 5. Stationary Aircraft Reticle / Horizon Reference (Doesn't rotate)
    ctx.strokeStyle = '#f59e0b';
    ctx.fillStyle = '#f59e0b';
    ctx.lineWidth = 3.5;

    // Left wing
    ctx.beginPath();
    ctx.moveTo(cx - 50, cy);
    ctx.lineTo(cx - 18, cy);
    ctx.lineTo(cx - 18, cy + 8);
    ctx.stroke();

    // Right wing
    ctx.beginPath();
    ctx.moveTo(cx + 18, cy + 8);
    ctx.lineTo(cx + 18, cy);
    ctx.lineTo(cx + 50, cy);
    ctx.stroke();

    // Center pip
    ctx.beginPath();
    ctx.arc(cx, cy, 3, 0, Math.PI * 2);
    ctx.fill();

    // 6. Roll Scale Index Triangle at Top
    ctx.fillStyle = '#f59e0b';
    ctx.beginPath();
    ctx.moveTo(cx, cy - r + 2);
    ctx.lineTo(cx - 6, cy - r + 14);
    ctx.lineTo(cx + 6, cy - r + 14);
    ctx.closePath();
    ctx.fill();
  }

  updateMotors(m1, m2, m3, m4) {
    const motors = [
      { id: 'm1', val: m1 },
      { id: 'm2', val: m2 },
      { id: 'm3', val: m3 },
      { id: 'm4', val: m4 }
    ];

    motors.forEach(m => {
      const elVal = document.getElementById(`val-${m.id}`);
      const elBar = document.getElementById(`bar-${m.id}`);
      if (elVal && elBar) {
        const rounded = Math.round(m.val);
        elVal.textContent = `${rounded}%`;
        elBar.style.width = `${Math.min(100, Math.max(0, rounded))}%`;
      }
    });
  }

  updateBattery(volts) {
    const el = document.getElementById('hud-battery');
    if (!el) return;
    el.textContent = `${volts.toFixed(2)} V`;
    if (volts > 0 && volts < 3.5) {
      el.style.color = '#ef4444'; // Red alert
    } else if (volts < 3.7) {
      el.style.color = '#f59e0b'; // Amber warning
    } else {
      el.style.color = '#10b981'; // Good
    }
  }

  updateRSSI(rssi) {
    const el = document.getElementById('hud-rssi');
    if (el) el.textContent = `${rssi} dBm`;
  }

  updatePing(ms) {
    const el = document.getElementById('hud-ping');
    if (el) el.textContent = `${ms} ms`;
  }
}
