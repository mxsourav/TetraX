import React, { useEffect, useRef, useState } from 'react';
import { Activity, Radio, Signal, Cpu } from 'lucide-react';

export interface NrfTelemetry {
  role: 'MASTER' | 'SLAVE';
  tx: number;
  rx: number;
  ok: number;
  lat: number;
  best: number;
  seq: number;
  time: number;
}

export function RFVisualizer({ telemetry }: { telemetry: NrfTelemetry | null }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const dataHistory = useRef<number[]>([]);
  const MAX_HISTORY = 100;

  useEffect(() => {
    if (!telemetry) return;
    
    // Add latency or success rate to history for graphing
    const val = telemetry.role === 'MASTER' ? telemetry.lat : Math.random() * 20 + 10; // Dummy activity for slave rx
    dataHistory.current.push(val);
    if (dataHistory.current.length > MAX_HISTORY) {
      dataHistory.current.shift();
    }

    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // Draw Grid
    ctx.strokeStyle = '#27272a'; // zinc-800
    ctx.lineWidth = 1;
    for(let i=0; i<canvas.width; i+=20) {
      ctx.beginPath(); ctx.moveTo(i, 0); ctx.lineTo(i, canvas.height); ctx.stroke();
    }
    for(let i=0; i<canvas.height; i+=20) {
      ctx.beginPath(); ctx.moveTo(0, i); ctx.lineTo(canvas.width, i); ctx.stroke();
    }

    // Draw Waveform
    if (dataHistory.current.length < 2) return;
    
    ctx.beginPath();
    ctx.strokeStyle = '#10b981'; // emerald-500
    ctx.lineWidth = 2;
    ctx.lineJoin = 'round';
    
    const stepX = canvas.width / (MAX_HISTORY - 1);
    
    dataHistory.current.forEach((val, i) => {
      const x = i * stepX;
      // Map latency (0-100ms usually) to canvas height
      const mappedY = Math.max(5, canvas.height - (val / 100) * canvas.height);
      const y = Math.min(canvas.height - 5, mappedY);
      
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    
    ctx.stroke();

    // Gradient fill under the line
    const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
    gradient.addColorStop(0, 'rgba(16, 185, 129, 0.2)');
    gradient.addColorStop(1, 'rgba(16, 185, 129, 0)');
    
    ctx.lineTo(canvas.width, canvas.height);
    ctx.lineTo(0, canvas.height);
    ctx.fillStyle = gradient;
    ctx.fill();

  }, [telemetry]);

  if (!telemetry) {
    return (
      <div className="panel-hardware p-4 flex flex-col items-center justify-center h-48 border border-zinc-800 rounded-xl bg-black/40">
        <Radio className="w-8 h-8 text-zinc-600 mb-2 animate-pulse" />
        <span className="text-xs font-mono font-bold text-zinc-500">AWAITING NRF LINK TELEMETRY...</span>
      </div>
    );
  }

  return (
    <div className="panel-hardware p-4 flex flex-col h-48 border border-emerald-900/30 rounded-xl bg-black/40 relative overflow-hidden group mb-4">
      <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top,_var(--tw-gradient-stops))] from-emerald-900/20 via-transparent to-transparent pointer-events-none" />
      
      {/* Header */}
      <div className="flex justify-between items-center mb-3 z-10 shrink-0">
        <div className="flex items-center gap-2">
          <Activity className="w-4 h-4 text-emerald-500 animate-pulse" />
          <h2 className="text-[10px] font-mono font-bold tracking-widest text-emerald-500">
            RF VISUALIZER
          </h2>
        </div>
        <div className="px-2 py-0.5 bg-emerald-500/10 border border-emerald-500/30 rounded text-[9px] font-mono text-emerald-400 font-bold">
          {telemetry.role} MODE
        </div>
      </div>

      {/* Main Content Area */}
      <div className="flex flex-1 gap-4 z-10 min-h-0">
        {/* Graph */}
        <div className="flex-1 relative border border-zinc-800/80 rounded bg-[#0a0a0c]">
          <canvas 
            ref={canvasRef}
            width={400}
            height={100}
            className="absolute inset-0 w-full h-full"
            style={{ imageRendering: 'pixelated' }}
          />
        </div>

        {/* Stats Panel */}
        <div className="w-32 flex flex-col justify-between shrink-0">
          {telemetry.role === 'MASTER' ? (
            <>
              <StatBox label="TX PACKETS" value={telemetry.tx.toString()} color="text-zinc-300" />
              <StatBox label="OK REPLIES" value={telemetry.ok.toString()} color="text-emerald-400" />
              <StatBox label="LATENCY" value={`${telemetry.lat}ms`} color="text-amber-400" />
              <StatBox label="BEST PING" value={`${telemetry.best}ms`} color="text-blue-400" />
            </>
          ) : (
            <>
              <StatBox label="RX PACKETS" value={telemetry.rx.toString()} color="text-emerald-400" />
              <StatBox label="ACK SENT" value={telemetry.ok.toString()} color="text-blue-400" />
              <StatBox label="SEQUENCE" value={`#${telemetry.seq}`} color="text-zinc-300" />
              <StatBox label="STATUS" value="LISTENING" color="text-amber-400" />
            </>
          )}
        </div>
      </div>
    </div>
  );
}

function StatBox({ label, value, color }: { label: string, value: string, color: string }) {
  return (
    <div className="flex flex-col bg-zinc-900/50 rounded px-2 py-1 border border-zinc-800/50">
      <span className="text-[8px] font-mono text-zinc-500">{label}</span>
      <span className={`text-xs font-mono font-bold ${color}`}>{value}</span>
    </div>
  );
}
