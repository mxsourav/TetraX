// serial.js
// Handles Web Serial API communication and stream parsing

class SerialManager {
    constructor() {
        this.port = null;
        this.reader = null;
        this.writer = null;
        this.keepReading = true;
        
        // Callbacks for other modules to hook into
        this.onLogParsed = null;
        this.onOledFrame = null;
        this.onConnect = null;
        this.onDisconnect = null;
        
        // Buffer for partial lines
        this.buffer = '';
    }

    async connect() {
        if (!('serial' in navigator)) {
            alert('Web Serial API is not supported in this browser. Please use Chrome/Edge.');
            return;
        }

        try {
            this.port = await navigator.serial.requestPort();
            await this.port.open({ baudRate: 115200 });
            
            this.keepReading = true;
            this.writer = this.port.writable.getWriter();
            
            if (this.onConnect) this.onConnect();
            
            this.readLoop();
        } catch (err) {
            console.error('Serial connection failed:', err);
            if (this.onDisconnect) this.onDisconnect();
        }
    }

    async disconnect() {
        this.keepReading = false;
        
        if (this.reader) {
            await this.reader.cancel();
            this.reader = null;
        }
        
        if (this.writer) {
            await this.writer.close();
            this.writer = null;
        }
        
        if (this.port) {
            await this.port.close();
            this.port = null;
        }
        
        if (this.onDisconnect) this.onDisconnect();
    }

    async write(data) {
        if (this.writer) {
            const encoder = new TextEncoder();
            await this.writer.write(encoder.encode(data));
        }
    }

    async readLoop() {
        while (this.port && this.port.readable && this.keepReading) {
            const decoder = new TextDecoder('utf-8', { fatal: false });
            this.reader = this.port.readable.getReader();
            
            try {
                while (true) {
                    const { value, done } = await this.reader.read();
                    if (done) break;
                    
                    if (value) {
                        this.processChunk(decoder.decode(value, { stream: true }));
                    }
                }
            } catch (error) {
                console.error("Read error:", error);
            } finally {
                if (this.reader) {
                    this.reader.releaseLock();
                    this.reader = null;
                }
            }
        }
    }

    processChunk(chunk) {
        this.buffer += chunk;
        let lines = this.buffer.split('\n');
        
        // Keep the last incomplete line in the buffer
        this.buffer = lines.pop() || '';
        
        for (let line of lines) {
            this.processLine(line.trim());
        }
    }

    processLine(line) {
        if (!line) return;

        // OLED Frame Handling
        if (line.startsWith('OLED|')) {
            const base64Data = line.substring(5);
            if (this.onOledFrame) this.onOledFrame(base64Data);
            return;
        }

        // Boot Garbage Filtering
        if (/[\x00-\x08\x0B\x0C\x0E-\x1F]/.test(line)) {
            // Unprintable binary characters detected - ignore line completely
            return;
        }
        if (line.includes("rst:0x") || line.includes("boot:")) {
            // Clean up raw ESP32 boot lines slightly, but keep them
            line = line.replace(/[^ -~]/g, ""); 
        }

        // Tag extraction and coloring
        const now = new Date();
        const timeStr = `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}:${String(now.getSeconds()).padStart(2, '0')}.${String(now.getMilliseconds()).padStart(3, '0')}`;

        let tag = '';
        let text = line;
        let cssClass = 'tag-default';
        let bold = false;

        const tagMatch = line.match(/^\[(.*?)\]\s*(.*)/);
        if (tagMatch) {
            tag = `[${tagMatch[1]}]`;
            text = tagMatch[2];
            const tagUpper = tagMatch[1].toUpperCase();

            // Semantic Highlighting
            if (tagUpper === 'SYSTEM' || tagUpper === 'INIT') cssClass = 'tag-system';
            else if (tagUpper === 'ERROR' || tagUpper === 'E') { cssClass = 'tag-error'; bold = true; }
            else if (tagUpper.includes('JAM')) { cssClass = 'tag-jammer'; bold = true; }
            else if (tagUpper.includes('SCAN')) cssClass = 'tag-scan';
            else if (tagUpper.includes('BEACON')) cssClass = 'tag-beacon';
            else if (tagUpper.includes('DEAUTH')) { cssClass = 'tag-deauth'; bold = true; }
            else if (tagUpper.includes('PORTAL')) cssClass = 'tag-portal';
        }

        if (this.onLogParsed) {
            this.onLogParsed({ time: timeStr, tag, text, cssClass, bold });
        }
    }
}

// Global instance
window.serialManager = new SerialManager();
