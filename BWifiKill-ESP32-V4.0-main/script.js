/* ═══ BWifiKill V4 — Companion Terminal Script ═══ */

const term   = document.getElementById('terminal');
const canvas = document.getElementById('oledCanvas');
const ctx    = canvas.getContext('2d', { willReadFrequently: false });

// Disable smoothing for sharp SSD1306 pixels
ctx.imageSmoothingEnabled = false;
if (ctx.mozImageSmoothingEnabled !== undefined) ctx.mozImageSmoothingEnabled = false;
if (ctx.webkitImageSmoothingEnabled !== undefined) ctx.webkitImageSmoothingEnabled = false;

let port, reader, outputStream;
let isConnected = false;
let isPaused    = false;

let frameBuffer    = '';
let receivingFrame = false;
let lastDecodedBinary = null;

// Default OLED tint: #00FFFB
let currentTint = { r: 0, g: 255, b: 251 };

// ─── INIT: Draw idle OLED state ───
function initOled() {
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, 128, 64);

    // Faint scanlines
    ctx.strokeStyle = 'rgba(0,255,251,0.03)';
    ctx.lineWidth = 1;
    for (let y = 0; y < 64; y += 2) {
        ctx.beginPath();
        ctx.moveTo(0, y + 0.5);
        ctx.lineTo(128, y + 0.5);
        ctx.stroke();
    }
}
initOled();

// ─── TERMINAL ───
function appendTerminal(text, cls) {
    if (isPaused) return;
    const atBottom = term.scrollTop >= term.scrollHeight - term.clientHeight - 10;

    const ts = new Date().toLocaleTimeString('en-US', { hour12: false });
    const line = document.createElement('div');
    line.innerHTML = `<span class="log-ts">[${ts}]</span> ${text}`;
    if (cls) line.classList.add(cls);
    term.appendChild(line);

    // Cap at 400 lines
    while (term.childElementCount > 400) term.firstElementChild.remove();

    if (atBottom) term.scrollTop = term.scrollHeight;
}

// Clear & Pause
document.getElementById('btnClearLog').addEventListener('click', () => { term.innerHTML = ''; });
document.getElementById('btnPauseLog').addEventListener('click', function () {
    isPaused = !isPaused;
    this.style.color = isPaused ? '#ffcc33' : '';
});

// ─── COLOR PICKER ───
const valR = document.getElementById('valR');
const valG = document.getElementById('valG');
const valB = document.getElementById('valB');

function setColor(r, g, b) {
    currentTint = { r, g, b };
    valR.textContent = r;
    valG.textContent = g;
    valB.textContent = b;
    if (lastDecodedBinary) renderFromBinary(lastDecodedBinary);
}

function hexToRgb(hex) {
    const m = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return m ? { r: +('0x'+m[1]), g: +('0x'+m[2]), b: +('0x'+m[3]) } : { r:0, g:255, b:251 };
}

// Preset buttons
document.querySelectorAll('.preset').forEach(btn => {
    btn.addEventListener('click', e => {
        document.querySelectorAll('.preset').forEach(b => b.classList.remove('active'));
        e.currentTarget.classList.add('active');
        const { r, g, b } = hexToRgb(e.currentTarget.dataset.color);
        setColor(r, g, b);
    });
});

// Color wheel click
document.getElementById('colorWheel').addEventListener('click', e => {
    const rect = e.currentTarget.getBoundingClientRect();
    const cx = rect.width / 2, cy = rect.height / 2;
    const x = e.clientX - rect.left - cx;
    const y = e.clientY - rect.top  - cy;
    const dist = Math.sqrt(x*x + y*y);
    if (dist > cx) return;

    let angle = Math.atan2(y, x) * (180 / Math.PI) + 90;
    if (angle < 0) angle += 360;

    const h = angle / 360, s = 1, v = 1;
    const i = Math.floor(h * 6), f = h * 6 - i;
    const p = 0, q = 1 - f, t = f;
    let r, g, b;
    switch (i % 6) {
        case 0: r=1; g=t; b=0; break; case 1: r=q; g=1; b=0; break;
        case 2: r=0; g=1; b=t; break; case 3: r=0; g=q; b=1; break;
        case 4: r=t; g=0; b=1; break; case 5: r=1; g=0; b=q; break;
    }
    setColor(Math.round(r*255), Math.round(g*255), Math.round(b*255));

    const dot = document.getElementById('wheelDot');
    dot.style.left = (e.clientX - rect.left) + 'px';
    dot.style.top  = (e.clientY - rect.top) + 'px';
    dot.style.transform = 'translate(-50%,-50%)';

    document.querySelectorAll('.preset').forEach(b => b.classList.remove('active'));
});

// Reset color
document.getElementById('btnResetColor').addEventListener('click', () => {
    setColor(0, 255, 251);
    document.querySelectorAll('.preset').forEach(b => b.classList.remove('active'));
    document.querySelector('.preset[data-color="#00fffb"]').classList.add('active');
    const dot = document.getElementById('wheelDot');
    dot.style.left = '50%'; dot.style.top = '4px'; dot.style.transform = 'translateX(-50%)';
});

// ─── SERIAL CONNECTION ───
document.getElementById('btnConnect').addEventListener('click', async () => {
    if (isConnected) return;
    try {
        port = await navigator.serial.requestPort();
        await port.open({ baudRate: 115200 });

        const enc = new TextEncoderStream();
        outputStream = enc.writable;
        enc.readable.pipeTo(port.writable);

        const dec = new TextDecoderStream();
        port.readable.pipeTo(dec.writable);
        reader = dec.readable.getReader();

        isConnected = true;
        updateStatus('CONNECTED');
        appendTerminal('[SYSTEM] Connected to serial port.', 'log-sys');
        readLoop();
    } catch (e) {
        updateStatus('ERROR');
        appendTerminal('[ERROR] ' + e.message, 'log-err');
    }
});

document.getElementById('btnDisconnect').addEventListener('click', () => {
    if (!isConnected) return;
    isConnected = false;
    updateStatus('DISCONNECTED');
    updateIP('--.--.--.--');
    updateUptime(0);
    initOled(); // Clear OLED
    appendTerminal('[SYSTEM] Disconnected.', 'log-sys');
});

// ─── TELEMETRY & UI UPDATERS ───
function updateStatus(state) {
    const disp = document.getElementById('statusDisplay');
    const dot = document.getElementById('statusDot');
    disp.innerHTML = '';
    disp.appendChild(dot);
    disp.appendChild(document.createTextNode(state));
    disp.className = 'stat-val';
    dot.className = 'dot';
    
    if (state === 'CONNECTED') { disp.classList.add('stat-green'); dot.classList.add('stat-green'); }
    else if (state === 'CONNECTING') { disp.classList.add('stat-yellow'); dot.classList.add('stat-yellow'); }
    else { disp.classList.add('stat-red'); dot.classList.add('stat-red'); }
}

function updateIP(ip) {
    document.getElementById('ipDisplay').innerText = ip;
}

function updateUptime(secStr) {
    const totalSecs = parseInt(secStr) || 0;
    const h = String(Math.floor(totalSecs / 3600)).padStart(2, '0');
    const m = String(Math.floor((totalSecs % 3600) / 60)).padStart(2, '0');
    const s = String(totalSecs % 60).padStart(2, '0');
    document.getElementById('uptimeCounter').innerText = `${h}:${m}:${s}`;
}

// ─── INDICATORS ───
let txT, rxT;
let irTxActive = false;

function setIrTx(active) {
    const el = document.getElementById('indicatorTX');
    if (active) {
        irTxActive = true;
        el.classList.add('active');
    } else {
        irTxActive = false;
        el.classList.remove('active');
    }
}

function triggerLed(type) {
    const el = document.getElementById('indicatorRX'); // only RX pulses now
    if (type === 'rx') {
        el.classList.add('active');
        clearTimeout(rxT);
        rxT = setTimeout(() => el.classList.remove('active'), 180);
    }
}

// ─── READ LOOP ───
async function readLoop() {
    let partial = '';
    while (isConnected) {
        try {
            const { value, done } = await reader.read();
            if (done) break;
            partial += value;
            const lines = partial.split(/\r?\n/);
            partial = lines.pop();

            for (const line of lines) {
                // Parse Telemetry Protocol
                if (line.startsWith('[STATUS]')) {
                    updateStatus(line.substring(9).trim());
                    continue;
                }
                if (line.startsWith('[NET]')) {
                    const parts = line.substring(6).split('=');
                    if (parts.length > 1) updateIP(parts[1].trim());
                    continue;
                }
                if (line.startsWith('[UPTIME]')) {
                    updateUptime(line.substring(9).trim());
                    continue;
                }
                if (line === '[IR_TX_START]') { setIrTx(true); continue; }
                if (line === '[IR_TX_STOP]') { setIrTx(false); continue; }
                if (line === '[IR_RX]') { triggerLed('rx'); continue; }

                if (line === '[MIRROR] START') {
                    receivingFrame = true; frameBuffer = '';
                } else if (line === '[MIRROR] END') {
                    receivingFrame = false;
                    decodeAndRender(frameBuffer);
                } else if (receivingFrame && line.startsWith('FRAME:')) {
                    frameBuffer += line.substring(6);
                } else {
                    let cls = '';
                    if (line.includes('[SYSTEM]')) cls = 'log-sys';
                    else if (line.includes('[ERROR]')) cls = 'log-err';
                    else if (line.includes('[WARN]'))  cls = 'log-warn';
                    else if (line.includes('[OK]'))    cls = 'log-ok';
                    else if (line.includes('[IR]'))    cls = 'log-ir';
                    else if (line.includes('[WIFI]'))  cls = 'log-wifi';
                    else if (line.includes('[NET]'))   cls = 'log-net';
                    
                    appendTerminal(line, cls);
                }
            }
        } catch (e) {
            updateStatus('ERROR');
            appendTerminal('[ERROR] ' + e.message, 'log-err');
            break;
        }
    }
}

async function sendCmd(cmd) {
    if (!isConnected || !outputStream) return;
    const w = outputStream.getWriter();
    await w.write(cmd + '\n');
    w.releaseLock();
}

// ─── FRAME DECODE ───
function decodeAndRender(b64) {
    try {
        lastDecodedBinary = atob(b64);
        if (lastDecodedBinary.length !== 1024) return;
        renderFromBinary(lastDecodedBinary);
    } catch (_) {}
}

function renderFromBinary(bin) {
    const img = ctx.createImageData(128, 64);
    const d = img.data;
    const { r, g, b } = currentTint;

    for (let page = 0; page < 8; page++) {
        for (let x = 0; x < 128; x++) {
            const byte = bin.charCodeAt(page * 128 + x);
            for (let bit = 0; bit < 8; bit++) {
                const y = page * 8 + bit;
                const idx = (y * 128 + x) * 4;
                if (byte & (1 << bit)) {
                    d[idx] = r; d[idx+1] = g; d[idx+2] = b; d[idx+3] = 255;
                } else {
                    d[idx] = 0; d[idx+1] = 0; d[idx+2] = 0; d[idx+3] = 255;
                }
            }
        }
    }
    ctx.putImageData(img, 0, 0);
}

// ─── BUTTONS ───
['Up','Down','Left','Right','Ok','Back'].forEach(name => {
    const el = document.getElementById('btn' + name);
    if (!el) return;
    const press   = () => { el.style.transform = 'translateY(2px)'; el.style.boxShadow = '0 1px 3px rgba(0,0,0,0.4)'; sendCmd('BTN:' + name.toUpperCase() + '=1'); };
    const release = () => { el.style.transform = ''; el.style.boxShadow = ''; sendCmd('BTN:' + name.toUpperCase() + '=0'); };
    el.addEventListener('mousedown', press);
    el.addEventListener('mouseup',   release);
    el.addEventListener('mouseleave', release);
    el.addEventListener('touchstart', e => { e.preventDefault(); press(); });
    el.addEventListener('touchend',   e => { e.preventDefault(); release(); });
});

// Reboot button
document.getElementById('btnReboot').addEventListener('click', () => sendCmd('REBOOT'));

// Uptime ticker
setInterval(() => {
    const el = document.getElementById('uptimeCounter');
    if (!el) return;
    const p = el.textContent.split(':');
    let h = +p[0], m = +p[1], s = +p[2];
    if (++s >= 60) { s = 0; if (++m >= 60) { m = 0; h++; } }
    el.textContent = [h,m,s].map(v => String(v).padStart(2,'0')).join(':');
}, 1000);

// Brightness slider
const rangeInput = document.querySelector('.range-input');
const rangeVal   = document.querySelector('.range-val');
if (rangeInput && rangeVal) {
    rangeInput.addEventListener('input', () => { rangeVal.textContent = rangeInput.value + '%'; });
}
