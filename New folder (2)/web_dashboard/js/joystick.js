/**
 * Virtual Joystick Controller (Touch / Mouse / Pointer)
 * Supports Mode 2 RC Quadcopter Flight Controls
 * Author: Alan Tomar (Jun 2024)
 */

class VirtualJoystick {
  constructor(options) {
    this.zone = document.getElementById(options.zoneId);
    this.puck = document.getElementById(options.puckId);
    this.isThrottleYaw = options.isThrottleYaw || false;
    this.onChange = options.onChange || (() => {});
    
    this.maxRadius = options.maxRadius || 80;
    this.deadzone = options.deadzone || 0.05; // 5% deadzone

    this.active = false;
    this.center = { x: 0, y: 0 };
    this.currentPos = { x: 0, y: 0 };

    // Default values
    this.valX = 0; // Normalized -1.0 to 1.0
    this.valY = 0; // Normalized -1.0 to 1.0 (or 0.0 to 1.0 for throttle)

    this.initListeners();
  }

  initListeners() {
    const handleStart = (e) => {
      this.active = true;
      const rect = this.zone.getBoundingClientRect();
      this.center = {
        x: rect.left + rect.width / 2,
        y: rect.top + rect.height / 2
      };
      const pt = e.touches ? e.touches[0] : e;
      this.handleMove(pt.clientX, pt.clientY);
    };

    const handleMoveEvent = (e) => {
      if (!this.active) return;
      const pt = e.touches ? e.touches[0] : e;
      this.handleMove(pt.clientX, pt.clientY);
    };

    const handleEnd = () => {
      if (!this.active) return;
      this.active = false;
      this.resetPosition();
    };

    this.zone.addEventListener('pointerdown', handleStart);
    window.addEventListener('pointermove', handleMoveEvent);
    window.addEventListener('pointerup', handleEnd);
    window.addEventListener('pointercancel', handleEnd);
  }

  handleMove(clientX, clientY) {
    let dx = clientX - this.center.x;
    let dy = clientY - this.center.y;
    const distance = Math.hypot(dx, dy);

    if (distance > this.maxRadius) {
      dx = (dx / distance) * this.maxRadius;
      dy = (dy / distance) * this.maxRadius;
    }

    this.currentPos = { x: dx, y: dy };
    this.puck.style.transform = `translate(${dx}px, ${dy}px)`;

    let normX = dx / this.maxRadius;
    let normY = -(dy / this.maxRadius); // Invert Y so up is positive

    // Apply deadzone
    if (Math.abs(normX) < this.deadzone) normX = 0;
    if (Math.abs(normY) < this.deadzone) normY = 0;

    if (this.isThrottleYaw) {
      // X = Yaw (-1 to 1), Y = Throttle (0 to 100)
      this.valX = normX;
      this.valY = Math.max(0, Math.min(100, (normY + 1) * 50));
    } else {
      // X = Roll (-1 to 1), Y = Pitch (-1 to 1)
      this.valX = normX;
      this.valY = normY;
    }

    this.onChange({ x: this.valX, y: this.valY });
  }

  resetPosition() {
    if (!this.isThrottleYaw) {
      // Right stick: springs fully back to center (0, 0)
      this.puck.style.transform = `translate(0px, 0px)`;
      this.valX = 0;
      this.valY = 0;
      this.onChange({ x: 0, y: 0 });
    } else {
      // Left stick: Yaw springs back to 0, but throttle stays at current position
      const stayDy = -((this.valY / 50) - 1) * this.maxRadius;
      this.puck.style.transform = `translate(0px, ${stayDy}px)`;
      this.valX = 0;
      this.onChange({ x: 0, y: this.valY });
    }
  }

  setThrottleDirect(valPercent) {
    if (!this.isThrottleYaw) return;
    this.valY = Math.max(0, Math.min(100, valPercent));
    const stayDy = -((this.valY / 50) - 1) * this.maxRadius;
    this.puck.style.transform = `translate(0px, ${stayDy}px)`;
    this.onChange({ x: this.valX, y: this.valY });
  }
}
