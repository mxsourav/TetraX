// app.js
// Orchestrates the UI logic and binds the hardware modules

document.addEventListener('DOMContentLoaded', () => {
    const sm = window.serialManager;
    const tm = window.terminalManager;
    const oled = window.oledEngine;

    // --- Serial Event Hookups ---
    sm.onLogParsed = (logObj) => {
        tm.appendLog(logObj);
    };

    sm.onOledFrame = (base64) => {
        oled.drawFrame(base64);
    };

    sm.onConnect = () => {
        const badge = document.getElementById('connection-badge');
        badge.innerText = 'CONNECTED';
        badge.className = 'px-2 py-0.5 rounded text-[10px] font-black tracking-widest bg-emerald-950 text-emerald-400 border border-emerald-900 uppercase shadow-[0_0_10px_rgba(52,211,153,0.3)]';
        
        document.getElementById('btn-connect').classList.replace('text-emerald-600', 'text-zinc-500');
        document.getElementById('btn-disconnect').classList.replace('text-zinc-500', 'text-red-600');
    };

    sm.onDisconnect = () => {
        const badge = document.getElementById('connection-badge');
        badge.innerText = 'DISCONNECTED';
        badge.className = 'px-2 py-0.5 rounded text-[10px] font-black tracking-widest bg-zinc-900 text-zinc-500 border border-zinc-800 uppercase shadow-inner';
        
        document.getElementById('btn-connect').classList.replace('text-zinc-500', 'text-emerald-600');
        document.getElementById('btn-disconnect').classList.replace('text-red-600', 'text-zinc-500');
    };

    // --- UI Controls: Terminal ---
    document.getElementById('btn-clear-log').addEventListener('click', () => tm.clear());
    
    document.getElementById('btn-toggle-time').addEventListener('click', (e) => {
        const isActive = tm.toggleTimestamps();
        if (isActive) e.target.classList.add('active');
        else e.target.classList.remove('active');
    });

    document.getElementById('btn-toggle-pause').addEventListener('click', (e) => {
        const isPaused = tm.togglePause();
        if (isPaused) {
            e.target.classList.add('active');
            e.target.innerHTML = `<i data-lucide="play" class="w-3 h-3"></i> RESUME`;
        } else {
            e.target.classList.remove('active');
            e.target.innerHTML = `<i data-lucide="pause" class="w-3 h-3"></i> PAUSE`;
        }
        lucide.createIcons(); // Re-render icon
    });

    // --- UI Controls: Connection ---
    document.getElementById('btn-connect').addEventListener('click', () => sm.connect());
    document.getElementById('btn-disconnect').addEventListener('click', () => sm.disconnect());
    document.getElementById('btn-reboot').addEventListener('click', () => sm.write('CMD_REBOOT\n'));

    // --- UI Controls: OLED Engine ---
    const colorPreview = document.getElementById('color-preview');
    const rgbR = document.getElementById('rgb-r');
    const rgbG = document.getElementById('rgb-g');
    const rgbB = document.getElementById('rgb-b');

    function updateColorUI(hex) {
        colorPreview.style.backgroundColor = hex;
        colorPreview.style.boxShadow = `0 0 15px ${hex}80`; // Add glow
        
        const r = parseInt(hex.slice(1, 3), 16);
        const g = parseInt(hex.slice(3, 5), 16);
        const b = parseInt(hex.slice(5, 7), 16);
        
        rgbR.innerText = `R: ${r.toString().padStart(3, '0')}`;
        rgbG.innerText = `G: ${g.toString().padStart(3, '0')}`;
        rgbB.innerText = `B: ${b.toString().padStart(3, '0')}`;
        
        oled.setColor(hex);
    }

    // Color Picker Wheel
    document.getElementById('color-picker').addEventListener('input', (e) => {
        updateColorUI(e.target.value);
    });

    // Preset Buttons
    document.querySelectorAll('.color-preset').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const hex = e.target.getAttribute('data-color');
            document.getElementById('color-picker').value = hex;
            updateColorUI(hex);
        });
    });

    // Invert Toggle
    document.getElementById('invert-toggle').addEventListener('change', (e) => {
        const dot = document.getElementById('invert-dot');
        const parent = dot.parentElement;
        oled.setInvert(e.target.checked);
        
        if (e.target.checked) {
            dot.style.transform = 'translateX(16px)';
            dot.style.backgroundColor = '#18181b'; // zinc-900
            parent.style.backgroundColor = '#d4d4d8'; // zinc-300
        } else {
            dot.style.transform = 'translateX(0)';
            dot.style.backgroundColor = '#71717a'; // zinc-500
            parent.style.backgroundColor = '#18181b'; // zinc-900
        }
    });

    // --- Hardware Buttons Mapping ---
    // Macros
    document.querySelectorAll('.hardware-macro-btn').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const macro = e.currentTarget.getAttribute('data-macro');
            sm.write(macro + '\n');
        });
    });

    // D-Pad and Actions
    document.querySelectorAll('.hardware-dpad-btn, .hardware-action-btn').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const b = e.currentTarget.getAttribute('data-btn');
            sm.write('BTN_' + b + '\n');
            
            // Trigger tactical TX LED blink
            const txLed = document.getElementById('tx-led');
            txLed.classList.replace('bg-cyan-950', 'bg-cyan-400');
            txLed.style.boxShadow = '0 0 8px #22d3ee';
            setTimeout(() => {
                txLed.classList.replace('bg-cyan-400', 'bg-cyan-950');
                txLed.style.boxShadow = 'none';
            }, 50);
        });
    });

    // --- Diag Level Mapping ---
    document.getElementById('diag-level').addEventListener('change', (e) => {
        sm.write('SET_LOG_LEVEL_' + e.target.value + '\n');
    });

    // Init state
    sm.onDisconnect();
});
