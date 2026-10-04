import React, { useState, useEffect } from 'react';
import { Activity, ShieldCheck, TrendingUp, Wallet, Layers, Cpu } from 'lucide-react';


interface UiLevel {
  price: number;
  volume: number;
}

interface TelemetryData {
  marketPrice: number;
  bestBid: number;
  bestAsk: number;
  cashBalance: number;
  netShares: number;
  throughput: number;
  p50LatencyUs: number;
  p999LatencyUs: number;
  bids: UiLevel[];
  asks: UiLevel[];
}

/// Start data to be replaced when it started running
const fallbackData: TelemetryData = {
  marketPrice: 100.00, bestBid: 99.95, bestAsk: 100.05, cashBalance: 1000000.00, netShares: 0,
  throughput: 0, p50LatencyUs: 0.0, p999LatencyUs: 0.0, bids: [], asks: []
};

export default function App() {
  const [data, setData] = useState<TelemetryData>(fallbackData);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchUpdates = async () => {
      try {
        /// Checks if they can get the stream data. if not shows an error message
        const response = await fetch('/dashboard_live_state.json');
        if (!response.ok) throw new Error("Waiting for trading simulator stream data...");
        const json = await response.json();
        setData(json);
        setError(null);
      } catch (err: any) {
        setError(err.message);
      }
    };

    const interval = setInterval(fetchUpdates, 500);
    return () => clearInterval(interval);
  }, []);

  return (
    <div style={{ padding: '24px', backgroundColor: '#0f172a', color: '#f8fafc', minHeight: '100vh', fontFamily: 'sans-serif' }}>
      {/* Note: I used TradingView as inspiration for colors */}
      {/* Header */}
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', borderBottom: '1px solid #334155', paddingBottom: '16px', marginBottom: '24px' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          <Activity size={28} color="#38bdf8" />
          <h1 style={{ fontSize: '24px', fontWeight: 'bold', margin: 0 }}>HFT Engine Live Telemetry Workstation</h1>
        </div>
        {error ? (
          <span style={{ backgroundColor: '#7f1d1d', color: '#fca5a5', padding: '6px 12px', borderRadius: '6px', fontSize: '13px' }}>{error}</span>
        ) : (
          <span style={{ backgroundColor: '#064e3b', color: '#6ee7b7', padding: '6px 12px', borderRadius: '6px', fontSize: '13px', display: 'flex', alignItems: 'center', gap: '6px' }}>
            <ShieldCheck size={16} /> Platform Streaming Connected
          </span>
        )}
      </header>

      {/* Show the core system and metrics */}
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(220px, 1fr))', gap: '16px', marginBottom: '24px' }}>
        
        <div style={{ backgroundColor: '#1e293b', padding: '16px', borderRadius: '8px', border: '1px solid #334155' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#94a3b8', marginBottom: '8px' }}><Cpu size={16} /> <b>Throughput</b></div>
          <span style={{ fontSize: '24px', fontWeight: 'bold', color: '#38bdf8' }}>{data.throughput.toLocaleString()} <span style={{ fontSize: '14px', fontWeight: 'normal' }}>ops/sec</span></span>
        </div>

        <div style={{ backgroundColor: '#1e293b', padding: '16px', borderRadius: '8px', border: '1px solid #334155' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#94a3b8', marginBottom: '8px' }}><Activity size={16} /> <b>p99.9 Tail Latency</b></div>
          <span style={{ fontSize: '24px', fontWeight: 'bold', color: '#fbbf24' }}>{data.p999LatencyUs} <span style={{ fontSize: '14px', fontWeight: 'normal' }}>us</span></span>
        </div>

        <div style={{ backgroundColor: '#1e293b', padding: '16px', borderRadius: '8px', border: '1px solid #334155' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#94a3b8', marginBottom: '8px' }}><TrendingUp size={16} /> <b>Market Price</b></div>
          <span style={{ fontSize: '24px', fontWeight: 'bold', color: '#f8fafc' }}>€{data.marketPrice.toFixed(2)}</span>
        </div>

        <div style={{ backgroundColor: '#1e293b', padding: '16px', borderRadius: '8px', border: '1px solid #334155' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#94a3b8', marginBottom: '8px' }}><Wallet size={16} /> <b>Portfolio Ledger</b></div>
          <span style={{ fontSize: '16px', fontWeight: 'bold', display: 'block', color: '#cbd5e1' }}>Cash: €{data.cashBalance.toFixed(2)}</span>
          <span style={{ fontSize: '14px', color: '#94a3b8' }}>Shares Owned: {data.netShares}</span>
        </div>

      </div>

      {/* Shows the order book depth */}
      <div style={{ backgroundColor: '#1e293b', padding: '20px', borderRadius: '8px', border: '1px solid #334155' }}>
        <h2 style={{ fontSize: '18px', fontWeight: 'bold', margin: '0 0 16px 0', display: 'flex', alignItems: 'center', gap: '8px' }}><Layers size={18} color="#38bdf8" /> Order Book Depth</h2>
        
        <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '24px' }}>
          
          {/* Bids column with green for buying */}
          <div>
            <h3 style={{ fontSize: '14px', color: '#4ade80', borderBottom: '1px solid #22c55e', paddingBottom: '6px', marginTop: 0 }}>Bids (Buy Demand)</h3>
            <table style={{ width: '100%', borderCollapse: 'collapse', marginTop: '8px' }}>
              <thead>
                <tr style={{ color: '#94a3b8', fontSize: '12px', textAlign: 'left' }}><th>Price</th><th style={{ textAlign: 'right' }}>Volume</th></tr>
              </thead>
              <tbody>
                {data.bids.map((b, i) => (
                  <tr key={i} style={{ borderBottom: '1px solid #334155', fontSize: '14px', height: '28px', color: '#bbf7d0' }}>
                    <td>€{b.price.toFixed(2)}</td><td style={{ textAlign: 'right', fontWeight: 'bold' }}>{b.volume}</td>
                  </tr>
                ))}
                {data.bids.length === 0 && <tr><td colSpan={2} style={{ color: '#64748b', fontSize: '13px', paddingTop: '8px' }}>No active bids resting.</td></tr>}
              </tbody>
            </table>
          </div>

          {/* Asks Column with red for sell offers */}
          <div>
            <h3 style={{ fontSize: '14px', color: '#f87171', borderBottom: '1px solid #ef4444', paddingBottom: '6px', marginTop: 0 }}>Asks (Sell Supply)</h3>
            <table style={{ width: '100%', borderCollapse: 'collapse', marginTop: '8px' }}>
              <thead>
                <tr style={{ color: '#94a3b8', fontSize: '12px', textAlign: 'left' }}><th>Price</th><th style={{ textAlign: 'right' }}>Volume</th></tr>
              </thead>
              <tbody>
                {data.asks.map((a, i) => (
                  <tr key={i} style={{ borderBottom: '1px solid #334155', fontSize: '14px', height: '28px', color: '#fecaca' }}>
                    <td>€{a.price.toFixed(2)}</td><td style={{ textAlign: 'right', fontWeight: 'bold' }}>{a.volume}</td>
                  </tr>
                ))}
                {data.asks.length === 0 && <tr><td colSpan={2} style={{ color: '#64748b', fontSize: '13px', paddingTop: '8px' }}>No active asks resting.</td></tr>}
              </tbody>
            </table>
          </div>

        </div>
      </div>

    </div>
  );
}
