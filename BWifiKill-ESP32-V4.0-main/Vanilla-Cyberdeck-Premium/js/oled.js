// oled.js
// Advanced multi-layer Canvas engine for rendering OLED frames with phosphor persistence and bloom

class OledEngine {
    constructor() {
        this.baseCanvas = document.getElementById('oled-base');
        this.phosphorCanvas = document.getElementById('oled-phosphor');
        
        this.baseCtx = this.baseCanvas.getContext('2d', { alpha: true });
        this.phosphorCtx = this.phosphorCanvas.getContext('2d', { alpha: true });
        
        this.width = 128;
        this.height = 64;
        
        this.rgbColor = [0, 255, 255]; // Cyan default
        this.inverted = false;
        
        this.fpsCounter = document.getElementById('fps-counter');
        this.frames = 0;
        this.lastFpsTime = performance.now();
        
        // Setup Phosphor Fade Loop
        this.phosphorFadeRate = 0.85; // How fast trails fade (0 = instant, 1 = never)
        requestAnimationFrame(() => this.renderLoop());
    }

    setColor(hex) {
        // Convert hex to rgb
        const r = parseInt(hex.slice(1, 3), 16);
        const g = parseInt(hex.slice(3, 5), 16);
        const b = parseInt(hex.slice(5, 7), 16);
        this.rgbColor = [r, g, b];
        
        // Redraw current frame immediately to reflect new color if needed
        // (Handled automatically on next incoming frame)
    }

    setInvert(invert) {
        this.inverted = invert;
    }

    drawFrame(base64Data) {
        try {
            const binaryString = atob(base64Data);
            const bytes = new Uint8Array(binaryString.length);
            for (let i = 0; i < binaryString.length; i++) {
                bytes[i] = binaryString.charCodeAt(i);
            }

            const imgData = this.baseCtx.createImageData(this.width, this.height);
            const data = imgData.data;

            let byteIdx = 0;
            const [r, g, b] = this.rgbColor;

            for (let page = 0; page < 8; page++) {
                for (let x = 0; x < this.width; x++) {
                    const byte = bytes[byteIdx++];
                    for (let bit = 0; bit < 8; bit++) {
                        const y = page * 8 + bit;
                        const idx = (y * this.width + x) * 4;
                        
                        let pixelOn = (byte & (1 << bit)) !== 0;
                        if (this.inverted) pixelOn = !pixelOn;

                        if (pixelOn) {
                            data[idx] = r;
                            data[idx + 1] = g;
                            data[idx + 2] = b;
                            data[idx + 3] = 255;
                        } else {
                            data[idx] = 0;
                            data[idx + 1] = 0;
                            data[idx + 2] = 0;
                            data[idx + 3] = 0; // Transparent background
                        }
                    }
                }
            }

            // Draw to base layer
            this.baseCtx.putImageData(imgData, 0, 0);

            // Stamp onto phosphor layer for ghosting/persistence
            // Phosphor layer uses globalAlpha to blend new frames over old ones
            this.phosphorCtx.globalAlpha = 1.0;
            this.phosphorCtx.drawImage(this.baseCanvas, 0, 0);

            this.frames++;
        } catch (e) {
            console.error("OLED Decode Error:", e);
        }
    }

    renderLoop() {
        // Calculate FPS
        const now = performance.now();
        if (now - this.lastFpsTime >= 1000) {
            if (this.fpsCounter) {
                this.fpsCounter.innerText = `${this.frames} FPS`;
            }
            this.frames = 0;
            this.lastFpsTime = now;
        }

        // Fade out the phosphor canvas over time to create trails
        this.phosphorCtx.fillStyle = `rgba(0, 0, 0, ${1.0 - this.phosphorFadeRate})`;
        this.phosphorCtx.globalCompositeOperation = 'destination-out';
        this.phosphorCtx.fillRect(0, 0, this.width, this.height);
        this.phosphorCtx.globalCompositeOperation = 'source-over';

        requestAnimationFrame(() => this.renderLoop());
    }
}

window.oledEngine = new OledEngine();
