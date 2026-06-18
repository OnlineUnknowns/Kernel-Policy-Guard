<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>KernelPolicyGuard</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;700&family=Inter:wght@300;400;500;600;700;800;900&display=swap');

*{margin:0;padding:0;box-sizing:border-box}
:root{
  --green:#00ff88;--blue:#0088ff;--purple:#8844ff;--red:#ff4444;--orange:#ff8800;
  --bg:#060810;--bg2:#0d1117;--bg3:#161b22;--border:#21262d;
  --text:#e6edf3;--muted:#7d8590;
}
html{scroll-behavior:smooth}
body{background:var(--bg);color:var(--text);font-family:'Inter',sans-serif;overflow-x:hidden}

/* ── STARS ── */
#stars{position:fixed;top:0;left:0;width:100%;height:100%;pointer-events:none;z-index:0}
.star{position:absolute;background:#fff;border-radius:50%;animation:twinkle var(--d,3s) infinite alternate}
@keyframes twinkle{from{opacity:.1}to{opacity:.9}}

/* ── HERO ── */
#hero{position:relative;min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center;padding:2rem;z-index:1}
.hero-glow{position:absolute;width:600px;height:600px;border-radius:50%;background:radial-gradient(circle,rgba(0,136,255,.15) 0%,transparent 70%);animation:pulse 4s ease-in-out infinite;pointer-events:none}
@keyframes pulse{0%,100%{transform:scale(1);opacity:.6}50%{transform:scale(1.1);opacity:1}}

.shield-wrap{position:relative;margin-bottom:2rem}
.shield-svg{width:120px;height:120px;filter:drop-shadow(0 0 30px rgba(0,255,136,.6));animation:float 3s ease-in-out infinite}
@keyframes float{0%,100%{transform:translateY(0)}50%{transform:translateY(-12px)}}
.shield-ring{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);width:160px;height:160px;border:1px solid rgba(0,255,136,.3);border-radius:50%;animation:spin-slow 8s linear infinite}
.shield-ring2{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);width:200px;height:200px;border:1px dashed rgba(0,136,255,.2);border-radius:50%;animation:spin-slow 12s linear infinite reverse}
@keyframes spin-slow{from{transform:translate(-50%,-50%) rotate(0deg)}to{transform:translate(-50%,-50%) rotate(360deg)}}

.hero-title{font-size:clamp(2.5rem,6vw,5rem);font-weight:900;letter-spacing:-2px;background:linear-gradient(135deg,#fff 0%,var(--green) 50%,var(--blue) 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;margin-bottom:.5rem;animation:fadeUp .8s ease both}
.hero-sub{font-size:1.1rem;color:var(--muted);margin-bottom:.5rem;animation:fadeUp .8s .1s ease both}
.hero-codename{font-family:'JetBrains Mono',monospace;font-size:.85rem;color:var(--purple);background:rgba(136,68,255,.1);border:1px solid rgba(136,68,255,.3);padding:.3rem .8rem;border-radius:20px;display:inline-block;margin-bottom:2rem;animation:fadeUp .8s .2s ease both}
@keyframes fadeUp{from{opacity:0;transform:translateY(20px)}to{opacity:1;transform:translateY(0)}}

.badges{display:flex;flex-wrap:wrap;gap:.6rem;justify-content:center;margin-bottom:3rem;animation:fadeUp .8s .3s ease both}
.badge{padding:.4rem 1rem;border-radius:20px;font-size:.78rem;font-weight:600;font-family:'JetBrains Mono',monospace;border:1px solid;transition:transform .2s,box-shadow .2s}
.badge:hover{transform:translateY(-2px);box-shadow:0 4px 20px currentColor}
.badge-green{color:var(--green);border-color:rgba(0,255,136,.4);background:rgba(0,255,136,.08)}
.badge-blue{color:var(--blue);border-color:rgba(0,136,255,.4);background:rgba(0,136,255,.08)}
.badge-orange{color:var(--orange);border-color:rgba(255,136,0,.4);background:rgba(255,136,0,.08)}
.badge-purple{color:var(--purple);border-color:rgba(136,68,255,.4);background:rgba(136,68,255,.08)}

.scroll-hint{color:var(--muted);font-size:.85rem;animation:bounce 2s infinite}
@keyframes bounce{0%,100%{transform:translateY(0)}50%{transform:translateY(6px)}}

/* ── SECTIONS ── */
section{position:relative;z-index:1;padding:5rem 2rem;max-width:1100px;margin:0 auto}
.section-label{font-family:'JetBrains Mono',monospace;font-size:.75rem;color:var(--green);letter-spacing:.15em;text-transform:uppercase;margin-bottom:.8rem;opacity:.8}
.section-title{font-size:clamp(1.8rem,4vw,2.8rem);font-weight:800;margin-bottom:1rem;letter-spacing:-1px}
.section-desc{color:var(--muted);font-size:1rem;line-height:1.7;max-width:600px;margin-bottom:3rem}

/* ── DIVIDER ── */
.divider{width:100%;height:1px;background:linear-gradient(90deg,transparent,var(--border),transparent);margin:1rem 0}

/* ── FEATURE CARDS ── */
.features-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:1.2rem}
.feat-card{background:var(--bg3);border:1px solid var(--border);border-radius:12px;padding:1.5rem;transition:transform .3s,border-color .3s,box-shadow .3s;cursor:default;opacity:0;transform:translateY(30px)}
.feat-card.visible{animation:cardIn .5s ease forwards}
@keyframes cardIn{to{opacity:1;transform:translateY(0)}}
.feat-card:hover{transform:translateY(-4px);border-color:var(--green);box-shadow:0 8px 40px rgba(0,255,136,.12)}
.feat-icon{font-size:2rem;margin-bottom:1rem;display:block}
.feat-title{font-size:1rem;font-weight:700;margin-bottom:.5rem;color:#fff}
.feat-desc{font-size:.88rem;color:var(--muted);line-height:1.6}
.feat-tag{display:inline-block;margin-top:.8rem;font-family:'JetBrains Mono',monospace;font-size:.72rem;color:var(--blue);background:rgba(0,136,255,.1);border:1px solid rgba(0,136,255,.25);padding:.2rem .6rem;border-radius:6px}

/* ── ARCH DIAGRAM ── */
.arch-wrap{background:var(--bg2);border:1px solid var(--border);border-radius:16px;padding:2rem;overflow:hidden;position:relative}
.arch-flow{display:flex;flex-direction:column;align-items:center;gap:0}
.arch-node{background:var(--bg3);border:1px solid var(--border);border-radius:10px;padding:.9rem 1.6rem;text-align:center;min-width:260px;position:relative;transition:border-color .3s,box-shadow .3s;cursor:default}
.arch-node:hover{border-color:var(--green);box-shadow:0 0 20px rgba(0,255,136,.15)}
.arch-node .node-title{font-size:.88rem;font-weight:700;color:#fff;margin-bottom:.2rem}
.arch-node .node-sub{font-size:.72rem;color:var(--muted);font-family:'JetBrains Mono',monospace}
.arch-node.green{border-color:rgba(0,255,136,.4);background:rgba(0,255,136,.05)}
.arch-node.blue{border-color:rgba(0,136,255,.4);background:rgba(0,136,255,.05)}
.arch-node.purple{border-color:rgba(136,68,255,.4);background:rgba(136,68,255,.05)}
.arch-arrow{width:2px;height:32px;background:linear-gradient(to bottom,var(--green),var(--blue));position:relative;margin:0 auto}
.arch-arrow::after{content:'▼';position:absolute;bottom:-10px;left:50%;transform:translateX(-50%);color:var(--blue);font-size:.7rem}
.arch-split{display:flex;gap:2rem;justify-content:center;width:100%}
.arch-split-line{display:flex;align-items:flex-start;gap:0;width:100%;justify-content:center;position:relative;padding-top:32px}
.arch-split-line::before{content:'';position:absolute;top:0;left:calc(50% - 150px);width:300px;height:2px;background:linear-gradient(90deg,var(--green),var(--blue))}
.arch-split-line::after{content:'';position:absolute;top:0;left:50%;width:2px;height:32px;background:var(--border)}

/* ── TERMINAL ── */
.terminal{background:#0d1117;border:1px solid #30363d;border-radius:12px;overflow:hidden;font-family:'JetBrains Mono',monospace;font-size:.82rem}
.term-bar{background:#161b22;padding:.7rem 1rem;display:flex;align-items:center;gap:.5rem;border-bottom:1px solid #30363d}
.term-dot{width:12px;height:12px;border-radius:50%}
.term-title{color:var(--muted);font-size:.75rem;margin-left:.5rem}
.term-body{padding:1.2rem;line-height:2}
.term-line{display:flex;align-items:center;gap:.5rem;opacity:0}
.term-line.typed{animation:typeIn .3s ease forwards}
@keyframes typeIn{to{opacity:1}}
.prompt{color:var(--green)}
.cmd{color:#e6edf3}
.out{color:var(--muted);padding-left:1.2rem}
.out.green{color:var(--green)}
.out.blue{color:var(--blue)}
.out.red{color:var(--red)}
.out.orange{color:var(--orange)}

/* ── STATS ── */
.stats-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:1.2rem;margin-bottom:3rem}
.stat-card{background:var(--bg3);border:1px solid var(--border);border-radius:12px;padding:1.5rem;text-align:center;position:relative;overflow:hidden}
.stat-card::before{content:'';position:absolute;top:0;left:0;right:0;height:2px;background:var(--accent,var(--green))}
.stat-num{font-size:2.2rem;font-weight:900;font-family:'JetBrains Mono',monospace;color:var(--accent,var(--green))}
.stat-label{font-size:.8rem;color:var(--muted);margin-top:.3rem}

/* ── SECURITY TABLE ── */
.sec-table{width:100%;border-collapse:collapse}
.sec-table th{text-align:left;padding:.8rem 1rem;color:var(--muted);font-size:.8rem;font-weight:600;text-transform:uppercase;letter-spacing:.1em;border-bottom:1px solid var(--border)}
.sec-table td{padding:.9rem 1rem;border-bottom:1px solid rgba(33,38,45,.5);font-size:.88rem;vertical-align:middle}
.sec-table tr:last-child td{border-bottom:none}
.sec-table tr:hover td{background:rgba(255,255,255,.02)}
.check{display:inline-flex;align-items:center;gap:.4rem;color:var(--green);font-weight:600;font-size:.82rem}
.check::before{content:'✓';width:20px;height:20px;background:rgba(0,255,136,.15);border:1px solid rgba(0,255,136,.3);border-radius:50%;display:inline-flex;align-items:center;justify-content:center;font-size:.7rem;flex-shrink:0}
code{font-family:'JetBrains Mono',monospace;font-size:.8rem;background:rgba(255,255,255,.06);padding:.15rem .4rem;border-radius:4px;color:#e6edf3}

/* ── TLV DIAGRAM ── */
.tlv-wrap{background:var(--bg2);border:1px solid var(--border);border-radius:16px;padding:2rem;font-family:'JetBrains Mono',monospace;font-size:.82rem}
.tlv-packet{border:1px solid rgba(0,136,255,.4);border-radius:8px;overflow:hidden;margin-bottom:1rem}
.tlv-row{display:flex;align-items:center;padding:.6rem 1rem;border-bottom:1px solid rgba(33,38,45,.8);transition:background .2s}
.tlv-row:last-child{border-bottom:none}
.tlv-row:hover{background:rgba(0,136,255,.05)}
.tlv-type{color:var(--orange);min-width:80px;font-weight:700}
.tlv-name{color:var(--blue);min-width:140px}
.tlv-desc{color:var(--muted);font-size:.78rem}
.tlv-header{background:rgba(0,136,255,.08)}
.tlv-divider-row{background:rgba(33,38,45,.6);color:var(--muted);font-size:.75rem;padding:.4rem 1rem;letter-spacing:.1em}

/* ── COMPAT TABLE ── */
.compat-grid{display:grid;gap:.8rem}
.compat-row{background:var(--bg3);border:1px solid var(--border);border-radius:8px;padding:.9rem 1.2rem;display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:.5rem;transition:border-color .3s}
.compat-row:hover{border-color:var(--green)}
.compat-kernel{font-family:'JetBrains Mono',monospace;font-weight:700;color:#fff;font-size:.9rem}
.compat-status{display:flex;align-items:center;gap:.5rem;font-size:.82rem;font-weight:600}
.compat-status.ok{color:var(--green)}
.compat-status.future{color:var(--purple)}
.compat-note{color:var(--muted);font-size:.8rem}

/* ── QUICKSTART STEPS ── */
.steps{display:flex;flex-direction:column;gap:1.5rem}
.step{display:flex;gap:1.2rem;align-items:flex-start}
.step-num{width:36px;height:36px;border-radius:50%;background:linear-gradient(135deg,var(--green),var(--blue));display:flex;align-items:center;justify-content:center;font-weight:900;font-size:.85rem;flex-shrink:0;color:#000}
.step-content{flex:1}
.step-title{font-weight:700;margin-bottom:.5rem;color:#fff}

/* ── FOOTER ── */
footer{text-align:center;padding:4rem 2rem;color:var(--muted);font-size:.85rem;position:relative;z-index:1}
footer .footer-logo{font-size:1.5rem;font-weight:900;background:linear-gradient(135deg,var(--green),var(--blue));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;margin-bottom:.5rem}
.footer-line{width:60px;height:2px;background:linear-gradient(90deg,var(--green),var(--blue));margin:.8rem auto}

/* ── SCAN LINE EFFECT ── */
.scanline{position:fixed;top:0;left:0;right:0;bottom:0;background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,.03) 2px,rgba(0,0,0,.03) 4px);pointer-events:none;z-index:9999;opacity:.4}

/* scrollbar */
::-webkit-scrollbar{width:6px}
::-webkit-scrollbar-track{background:var(--bg)}
::-webkit-scrollbar-thumb{background:var(--border);border-radius:3px}

/* reveal */
.reveal{opacity:0;transform:translateY(24px);transition:opacity .6s ease,transform .6s ease}
.reveal.visible{opacity:1;transform:none}
</style>
</head>
<body>

<div class="scanline"></div>
<canvas id="stars"></canvas>

<!-- HERO -->
<div id="hero">
  <div class="hero-glow"></div>
  <div class="shield-wrap">
    <div class="shield-ring"></div>
    <div class="shield-ring2"></div>
    <svg class="shield-svg" viewBox="0 0 100 110" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M50 5 L90 22 L90 55 C90 78 72 98 50 105 C28 98 10 78 10 55 L10 22 Z" fill="rgba(0,255,136,0.1)" stroke="url(#sg)" stroke-width="2"/>
      <defs>
        <linearGradient id="sg" x1="50" y1="5" x2="50" y2="105" gradientUnits="userSpaceOnUse">
          <stop offset="0%" stop-color="#00ff88"/>
          <stop offset="100%" stop-color="#0088ff"/>
        </linearGradient>
      </defs>
      <path d="M50 5 L90 22 L90 55 C90 78 72 98 50 105 C28 98 10 78 10 55 L10 22 Z" fill="none" stroke="url(#sg)" stroke-width="2.5"/>
      <path d="M35 52 L45 62 L65 42" stroke="#00ff88" stroke-width="4" stroke-linecap="round" stroke-linejoin="round"/>
    </svg>
  </div>
  <h1 class="hero-title">KernelPolicyGuard</h1>
  <p class="hero-sub">Dynamic LKM Security Framework · Real-Time Policy Enforcement · Cluster-Aware</p>
  <span class="hero-codename">// formerly: Chronos · Karenl</span>
  <div class="badges">
    <span class="badge badge-blue">Linux 5.10+</span>
    <span class="badge badge-green">GPLv2</span>
    <span class="badge badge-orange">NUMA · SMP</span>
    <span class="badge badge-purple">Enterprise Ready</span>
    <span class="badge badge-green">Production</span>
    <span class="badge badge-blue">fprobe · kprobe</span>
    <span class="badge badge-purple">Generic Netlink</span>
  </div>
  <p class="scroll-hint">↓ scroll to explore ↓</p>
</div>

<!-- STATS -->
<section>
  <div class="divider"></div>
  <div class="stats-grid reveal">
    <div class="stat-card" style="--accent:var(--green)">
      <div class="stat-num" id="cnt-nodes">0</div>
      <div class="stat-label">Max Cluster Nodes</div>
    </div>
    <div class="stat-card" style="--accent:var(--blue)">
      <div class="stat-num" id="cnt-rules">0</div>
      <div class="stat-label">Policy Rules per Set</div>
    </div>
    <div class="stat-card" style="--accent:var(--purple)">
      <div class="stat-num" id="cnt-cores">0</div>
      <div class="stat-label">CPU Cores Supported</div>
    </div>
    <div class="stat-card" style="--accent:var(--orange)">
      <div class="stat-num" id="cnt-dos">0</div>
      <div class="stat-label">DoS Event Cap</div>
    </div>
  </div>
  <div class="divider"></div>
</section>

<!-- FEATURES -->
<section>
  <div class="section-label reveal">✦ capabilities</div>
  <h2 class="section-title reveal">Key Features</h2>
  <div class="features-grid">
    <div class="feat-card"><span class="feat-icon">⚡</span><div class="feat-title">Zero-Copy Hooking</div><div class="feat-desc">fprobe (ftrace-based) for near-zero overhead interception at kernel function entry/exit points.</div><span class="feat-tag">fprobe · kprobe</span></div>
    <div class="feat-card"><span class="feat-icon">🧠</span><div class="feat-title">NUMA-Aware Workqueue</div><div class="feat-desc">Per-CPU dispatch via queue_work_on() maintaining cache locality across NUMA nodes with WQ_HIGHPRI.</div><span class="feat-tag">queue_work_on(cpu)</span></div>
    <div class="feat-card"><span class="feat-icon">📡</span><div class="feat-title">TLV Cluster Sync</div><div class="feat-desc">Type-Length-Value protocol over UDP with CRC32 integrity checks. Forward-compatible across protocol versions.</div><span class="feat-tag">UDP · TLV · CRC32</span></div>
    <div class="feat-card"><span class="feat-icon">🔐</span><div class="feat-title">Secure by Design</div><div class="feat-desc">Sender whitelisting, seqcount lock-free reads, strict GFP_ATOMIC rules, and rate-limited kernel logs.</div><span class="feat-tag">seqcount · GFP_ATOMIC</span></div>
    <div class="feat-card"><span class="feat-icon">🖥️</span><div class="feat-title">Dual Interface</div><div class="feat-desc">sysfs for manual admin control plus Generic Netlink for high-performance daemon integration and multicast events.</div><span class="feat-tag">sysfs · genl</span></div>
    <div class="feat-card"><span class="feat-icon">🔧</span><div class="feat-title">Dynamic Policy Engine</div><div class="feat-desc">64-rule sets with EQ/GT/LT/RANGE operators. First-match-wins evaluation with ALLOW, DENY, and LOG actions.</div><span class="feat-tag">64 rules · first-match</span></div>
  </div>
</section>

<!-- ARCHITECTURE -->
<section>
  <div class="section-label reveal">✦ architecture</div>
  <h2 class="section-title reveal">System Flow</h2>
  <p class="section-desc reveal">Every security event flows from kernel interception through NUMA-aware processing to policy evaluation and cluster synchronization.</p>
  <div class="arch-wrap reveal">
    <div class="arch-flow">
      <div class="arch-node green">
        <div class="node-title">👤 User Process</div>
        <div class="node-sub">open() · execve() · send() · fork()</div>
      </div>
      <div class="arch-arrow"></div>
      <div class="arch-node green">
        <div class="node-title">🔬 Fprobe / Kprobe  —  Atomic Context</div>
        <div class="node-sub">READ_ONCE(cfg.enabled) → queue_work_on(cpu, item)</div>
      </div>
      <div class="arch-arrow"></div>
      <div class="arch-node blue">
        <div class="node-title">⚙️ NUMA-Aware Workqueue  —  Process Context</div>
        <div class="node-sub">Per-CPU dispatch · DoS cap: MAX_PENDING = 1000</div>
      </div>
      <div class="arch-split-line"></div>
      <div class="arch-split">
        <div style="display:flex;flex-direction:column;align-items:center;gap:0">
          <div class="arch-node blue" style="min-width:220px">
            <div class="node-title">📜 Policy Engine</div>
            <div class="node-sub">64 rules · ALLOW / DENY / LOG</div>
          </div>
        </div>
        <div style="display:flex;flex-direction:column;align-items:center;gap:0">
          <div class="arch-node purple" style="min-width:220px">
            <div class="node-title">📡 Cluster Sync UDP</div>
            <div class="node-sub">TLV Protocol v1 · CRC32 + whitelist</div>
          </div>
        </div>
      </div>
      <div class="arch-arrow" style="background:linear-gradient(to bottom,var(--blue),var(--purple))"></div>
      <div class="arch-node purple">
        <div class="node-title">🌐 Generic Netlink  —  KERNEL_POLICY_GUARD</div>
        <div class="node-sub">GET_STATS · UPDATE_POLICY · STREAM_EVENTS multicast</div>
      </div>
    </div>
  </div>
</section>

<!-- TERMINAL QUICKSTART -->
<section>
  <div class="section-label reveal">✦ quick start</div>
  <h2 class="section-title reveal">Get Running in 60 Seconds</h2>
  <div class="terminal reveal" id="term">
    <div class="term-bar">
      <div class="term-dot" style="background:#ff5f57"></div>
      <div class="term-dot" style="background:#febc2e"></div>
      <div class="term-dot" style="background:#28c840"></div>
      <span class="term-title">root@enterprise-node-01 ~ KernelPolicyGuard</span>
    </div>
    <div class="term-body" id="term-body"></div>
  </div>
</section>

<!-- TLV PROTOCOL -->
<section>
  <div class="section-label reveal">✦ cluster protocol</div>
  <h2 class="section-title reveal">TLV Sync Protocol</h2>
  <p class="section-desc reveal">Forward-compatible UDP protocol. Unknown TLV types are silently skipped, enabling zero-downtime upgrades across mixed-version clusters.</p>
  <div class="tlv-wrap reveal">
    <div style="color:var(--muted);font-size:.78rem;margin-bottom:1rem;letter-spacing:.05em">PACKET STRUCTURE</div>
    <div class="tlv-packet">
      <div class="tlv-row tlv-header">
        <span class="tlv-type">uint16</span>
        <span class="tlv-name">total_len</span>
        <span class="tlv-desc">Total packet length in bytes</span>
      </div>
      <div class="tlv-row tlv-header">
        <span class="tlv-type">uint16</span>
        <span class="tlv-name">magic</span>
        <span class="tlv-desc" style="color:var(--orange)">0xC4D5  —  protocol identifier</span>
      </div>
      <div class="tlv-row tlv-header">
        <span class="tlv-type">uint8</span>
        <span class="tlv-name">version</span>
        <span class="tlv-desc" style="color:var(--green)">1  —  current protocol version</span>
      </div>
      <div class="tlv-divider-row">── TLV RECORDS ──────────────────────────────────</div>
      <div class="tlv-row">
        <span class="tlv-type">0x01</span>
        <span class="tlv-name" style="color:var(--green)">TYPE_NODE_ID</span>
        <span class="tlv-desc">Cluster node identifier (UUID)</span>
      </div>
      <div class="tlv-row">
        <span class="tlv-type">0x02</span>
        <span class="tlv-name" style="color:var(--blue)">TYPE_LOAD</span>
        <span class="tlv-desc">Current node load metric (u64)</span>
      </div>
      <div class="tlv-row">
        <span class="tlv-type">0x03</span>
        <span class="tlv-name" style="color:var(--red)">TYPE_BLACKLIST</span>
        <span class="tlv-desc">Blacklisted entity hash (SHA-256)</span>
      </div>
      <div class="tlv-row" style="opacity:.5;font-style:italic">
        <span class="tlv-type">0xNN</span>
        <span class="tlv-name" style="color:var(--muted)">UNKNOWN</span>
        <span class="tlv-desc">Silently skipped — future-proof</span>
      </div>
      <div class="tlv-divider-row">── CHECKSUM ─────────────────────────────────────</div>
      <div class="tlv-row tlv-header">
        <span class="tlv-type">uint32</span>
        <span class="tlv-name">crc32</span>
        <span class="tlv-desc" style="color:var(--purple)">Over entire TLV blob — integrity check</span>
      </div>
    </div>
  </div>
</section>

<!-- SECURITY HARDENING -->
<section>
  <div class="section-label reveal">✦ security</div>
  <h2 class="section-title reveal">Security Hardening</h2>
  <p class="section-desc reveal">Every component independently audited. Zero tolerance for locking violations in atomic context.</p>
  <div style="background:var(--bg3);border:1px solid var(--border);border-radius:16px;overflow:hidden" class="reveal">
    <table class="sec-table">
      <thead><tr><th>Component</th><th>Status</th><th>Detail</th></tr></thead>
      <tbody>
        <tr><td><code>Locking model</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">seqcount for lock-free reads; mutex only in sysfs write paths</td></tr>
        <tr><td><code>Atomic context</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">No mutex_lock() or GFP_KERNEL inside probe handlers</td></tr>
        <tr><td><code>Input sanitization</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">All sysfs stores use strscpy() with explicit range validation</td></tr>
        <tr><td><code>Cluster input</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">TLV parser enforces length + CRC32 checksum on every packet</td></tr>
        <tr><td><code>DoS protection</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">Pending work capped at MAX_PENDING_WORK = 1000 events</td></tr>
        <tr><td><code>Rate limiting</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">All probe-path printk wrapped with printk_ratelimit()</td></tr>
        <tr><td><code>Clean shutdown</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">kthread_stop() → flush_workqueue() → sock_release() in sequence</td></tr>
        <tr><td><code>Sender whitelist</code></td><td><span class="check">Verified</span></td><td style="color:var(--muted);font-size:.82rem">UDP source IPs validated against configured cluster peer list</td></tr>
      </tbody>
    </table>
  </div>
</section>

<!-- KERNEL COMPAT -->
<section>
  <div class="section-label reveal">✦ compatibility</div>
  <h2 class="section-title reveal">Kernel Version Support</h2>
  <div class="compat-grid reveal">
    <div class="compat-row"><span class="compat-kernel">Linux 5.10 LTS</span><span class="compat-status ok">● Supported</span><span class="compat-note">Minimum supported version</span></div>
    <div class="compat-row"><span class="compat-kernel">Linux 5.15 LTS</span><span class="compat-status ok">● Supported</span><span class="compat-note">Recommended for production</span></div>
    <div class="compat-row"><span class="compat-kernel">Linux 6.1 LTS</span><span class="compat-status ok">● Supported</span><span class="compat-note">Uses updated genl_register_family() API</span></div>
    <div class="compat-row"><span class="compat-kernel">Linux 6.6 LTS</span><span class="compat-status ok">● Supported</span><span class="compat-note">Full fprobe multi-handler support</span></div>
    <div class="compat-row" style="border-color:rgba(136,68,255,.3)"><span class="compat-kernel">Linux 6.x / 7.x</span><span class="compat-status future">◆ Future-proof</span><span class="compat-note">TLV + genl design ensures forward compatibility</span></div>
  </div>
</section>

<!-- FOOTER -->
<footer>
  <div class="footer-logo">🛡️ KernelPolicyGuard</div>
  <div class="footer-line"></div>
  <p>GNU General Public License v2 (only) · Built for Linux kernel 5.10+</p>
  <p style="margin-top:.5rem">Maintained by <strong style="color:var(--green)">KernelPolicyGuard Research</strong></p>
  <p style="margin-top:1.5rem;font-size:.75rem;opacity:.5">For enterprise deployment · Custom eBPF · Dedicated SLA · Multi-site cluster mesh</p>
</footer>

<script>
/* ── STARS ── */
const canvas=document.getElementById('stars');
const ctx=canvas.getContext('2d');
let W,H,stars=[];
function initStars(){
  W=canvas.width=window.innerWidth;
  H=canvas.height=window.innerHeight;
  stars=Array.from({length:150},()=>({
    x:Math.random()*W,y:Math.random()*H,
    r:Math.random()*1.2+.3,
    a:Math.random(),da:(Math.random()-.5)*.01
  }));
}
function drawStars(){
  ctx.clearRect(0,0,W,H);
  stars.forEach(s=>{
    s.a=Math.max(.05,Math.min(1,s.a+s.da));
    if(s.a<=.06||s.a>=.99)s.da*=-1;
    ctx.globalAlpha=s.a;
    ctx.fillStyle='#ffffff';
    ctx.beginPath();ctx.arc(s.x,s.y,s.r,0,Math.PI*2);ctx.fill();
  });
  ctx.globalAlpha=1;
  requestAnimationFrame(drawStars);
}
window.addEventListener('resize',initStars);
initStars();drawStars();

/* ── COUNT UP ── */
function countUp(el,target,suffix=''){
  let start=0,dur=1800,step=16;
  const inc=target/(dur/step);
  const t=setInterval(()=>{
    start=Math.min(start+inc,target);
    el.textContent=Math.round(start).toLocaleString()+suffix;
    if(start>=target)clearInterval(t);
  },step);
}

/* ── REVEAL OBSERVER ── */
const revealObs=new IntersectionObserver(entries=>{
  entries.forEach(e=>{
    if(e.isIntersecting){
      e.target.classList.add('visible');
      if(e.target.classList.contains('stats-grid')){
        countUp(document.getElementById('cnt-nodes'),1000,'+');
        countUp(document.getElementById('cnt-rules'),64);
        countUp(document.getElementById('cnt-cores'),128);
        countUp(document.getElementById('cnt-dos'),1000);
      }
      revealObs.unobserve(e.target);
    }
  });
},{threshold:.15});
document.querySelectorAll('.reveal').forEach(el=>revealObs.observe(el));

/* ── FEAT CARDS ── */
const cardObs=new IntersectionObserver(entries=>{
  entries.forEach((e,i)=>{
    if(e.isIntersecting){
      setTimeout(()=>e.target.classList.add('visible'),i*80);
      cardObs.unobserve(e.target);
    }
  });
},{threshold:.1});
document.querySelectorAll('.feat-card').forEach(c=>cardObs.observe(c));

/* ── TERMINAL TYPING ── */
const lines=[
  {type:'cmd',text:'make clean && make -j$(nproc)'},
  {type:'out green',text:'[KernelPolicyGuard] Build complete. Module ready.'},
  {type:'cmd',text:'sudo insmod KernelPolicyGuard.ko'},
  {type:'out green',text:'[  OK  ] Module loaded. Hooks active on 32 CPUs.'},
  {type:'cmd',text:'cat /sys/chronos/enabled'},
  {type:'out blue',text:'1'},
  {type:'cmd',text:"echo 'add inode open gt 5 deny' > /sys/chronos/rule_ctl"},
  {type:'out green',text:'[  OK  ] Rule added: inode.open > 5 → DENY'},
  {type:'cmd',text:'cat /sys/chronos/rule_count'},
  {type:'out blue',text:'1 / 64'},
  {type:'cmd',text:'cat /sys/chronos/dropped'},
  {type:'out orange',text:'247  (DoS protection active)'},
  {type:'cmd',text:'sudo rmmod KernelPolicyGuard'},
  {type:'out green',text:'[  OK  ] Clean shutdown. Workqueue flushed. Socket released.'},
];

const termObs=new IntersectionObserver(entries=>{
  if(entries[0].isIntersecting){
    termObs.disconnect();
    const body=document.getElementById('term-body');
    lines.forEach((l,i)=>{
      setTimeout(()=>{
        const div=document.createElement('div');
        div.className='term-line '+l.type+' typed';
        if(l.type==='cmd'){
          div.innerHTML=`<span class="prompt">root@node-01:~#</span><span class="cmd"> ${l.text}</span>`;
        } else {
          div.innerHTML=`<span class="${l.type}">${l.text}</span>`;
        }
        body.appendChild(div);
        body.scrollTop=body.scrollHeight;
      },i*420);
    });
  }
},{threshold:.3});
termObs.observe(document.getElementById('term'));
</script>
</body>
</html>
