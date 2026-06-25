// terminal.js
// Handles high-performance DOM manipulation for the serial log terminal

class TerminalManager {
    constructor() {
        this.viewport = document.getElementById('terminal-viewport');
        this.anchor = document.getElementById('terminal-anchor');
        
        this.showTimestamps = true;
        this.isPaused = false;
        this.autoScrollEnabled = true;
        
        this.MAX_LINES = 1500;
        this.lineCount = 0;

        this.setupScrollListener();
    }

    setupScrollListener() {
        this.viewport.addEventListener('scroll', () => {
            // Smart auto-scroll logic
            const isAtBottom = this.viewport.scrollHeight - this.viewport.scrollTop - this.viewport.clientHeight < 30;
            this.autoScrollEnabled = isAtBottom;
        });
    }

    appendLog(logObj) {
        if (this.isPaused) return;

        // Build DOM Element
        const lineDiv = document.createElement('div');
        lineDiv.className = 'log-line';

        let html = '';
        if (this.showTimestamps) {
            html += `<span class="log-time">${logObj.time}</span>`;
        }
        
        if (logObj.tag) {
            html += `<span class="log-tag ${logObj.cssClass}" ${logObj.bold ? 'style="font-weight: 900;"' : ''}>${logObj.tag}</span>`;
        }
        
        html += `<span class="log-text">${this.escapeHtml(logObj.text)}</span>`;
        
        lineDiv.innerHTML = html;

        // Insert before the anchor
        this.viewport.insertBefore(lineDiv, this.anchor);
        this.lineCount++;

        // Enforce max lines
        while (this.lineCount > this.MAX_LINES) {
            this.viewport.removeChild(this.viewport.firstChild);
            this.lineCount--;
        }

        // Apply scroll
        if (this.autoScrollEnabled) {
            this.anchor.scrollIntoView();
        }
    }

    clear() {
        // Remove all nodes except the anchor
        while (this.viewport.firstChild !== this.anchor) {
            this.viewport.removeChild(this.viewport.firstChild);
        }
        this.lineCount = 0;
    }

    toggleTimestamps() {
        this.showTimestamps = !this.showTimestamps;
        return this.showTimestamps;
    }

    togglePause() {
        this.isPaused = !this.isPaused;
        return this.isPaused;
    }

    escapeHtml(unsafe) {
        return unsafe
             .replace(/&/g, "&amp;")
             .replace(/</g, "&lt;")
             .replace(/>/g, "&gt;")
             .replace(/"/g, "&quot;")
             .replace(/'/g, "&#039;");
    }
}

window.terminalManager = new TerminalManager();
