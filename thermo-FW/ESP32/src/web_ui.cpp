#include "web_ui.h"

static const char WEB_UI[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="cs">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>THERMO</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: Arial, sans-serif; background: #1a1a2e; color: #eee; }

  .header {
    background: #16213e;
    padding: 15px 20px;
    border-bottom: 2px solid #0f3460;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .header h1 { font-size: 1.4em; color: #e94560; }
  .header .unit-info { font-size: 0.8em; color: #aaa; text-align: right; }

  .tabs {
    display: flex;
    background: #16213e;
    border-bottom: 1px solid #0f3460;
  }
  .tab {
    padding: 12px 20px;
    cursor: pointer;
    color: #aaa;
    border-bottom: 2px solid transparent;
    transition: all 0.2s;
  }
  .tab.active { color: #e94560; border-bottom-color: #e94560; }
  .tab:hover { color: #eee; }

  .content { display: none; padding: 20px; max-width: 800px; margin: 0 auto; }
  .content.active { display: block; }

  .card {
    background: #16213e;
    border-radius: 8px;
    padding: 15px;
    margin-bottom: 15px;
    border: 1px solid #0f3460;
  }
  .card h2 { font-size: 1em; color: #e94560; margin-bottom: 12px; }

  .sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 10px;
  }
  .sensor-item {
    background: #0f3460;
    border-radius: 6px;
    padding: 12px;
  }
  .sensor-name { font-size: 0.8em; color: #aaa; margin-bottom: 4px; }
  .sensor-value { font-size: 1.6em; font-weight: bold; color: #e94560; }
  .sensor-value.invalid { color: #555; }
  .sensor-rom { font-size: 0.65em; color: #666; margin-top: 4px; }
  .sensor-unnamed { border-left: 3px solid #e9a800; }

  .status-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 8px;
  }
  .status-item { font-size: 0.9em; }
  .status-label { color: #aaa; }
  .status-value { color: #eee; font-weight: bold; }
  .status-ok { color: #4caf50; }
  .status-err { color: #e94560; }

  .form-group { margin-bottom: 12px; }
  .form-group label { display: block; font-size: 0.85em; color: #aaa; margin-bottom: 4px; }
  .form-group input {
    width: 100%;
    padding: 8px 12px;
    background: #0f3460;
    border: 1px solid #1a4a8a;
    border-radius: 4px;
    color: #eee;
    font-size: 0.9em;
  }
  .form-group input:focus { outline: none; border-color: #e94560; }

  .btn {
    padding: 8px 20px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-size: 0.9em;
    transition: opacity 0.2s;
  }
  .btn:hover { opacity: 0.8; }
  .btn-primary { background: #e94560; color: white; }
  .btn-secondary { background: #0f3460; color: #eee; }

  .sensor-rename {
    display: flex;
    gap: 8px;
    margin-top: 8px;
  }
  .sensor-rename input {
    flex: 1;
    padding: 6px 10px;
    background: #0f3460;
    border: 1px solid #1a4a8a;
    border-radius: 4px;
    color: #eee;
    font-size: 0.85em;
  }

  .log-list { list-style: none; }
  .log-item {
    padding: 6px 10px;
    border-radius: 4px;
    margin-bottom: 4px;
    font-size: 0.8em;
    font-family: monospace;
  }
  .log-WARN     { background: #3a2a00; border-left: 3px solid #e9a800; }
  .log-ERROR    { background: #3a0000; border-left: 3px solid #e94560; }
  .log-CRITICAL { background: #5a0000; border-left: 3px solid #ff0000; }

  .wifi-list { list-style: none; }
  .wifi-item {
    padding: 8px 12px;
    background: #0f3460;
    border-radius: 4px;
    margin-bottom: 6px;
    cursor: pointer;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .wifi-item:hover { background: #1a4a8a; }
  .wifi-rssi { font-size: 0.8em; color: #aaa; }

  .msg { padding: 8px 12px; border-radius: 4px; margin-top: 10px; font-size: 0.85em; display: none; }
  .msg-ok  { background: #1a3a1a; color: #4caf50; }
  .msg-err { background: #3a1a1a; color: #e94560; }

  .refresh-info { font-size: 0.75em; color: #555; text-align: right; margin-bottom: 10px; }
</style>
</head>
<body>

<div class="header">
  <h1>🌡 THERMO</h1>
  <div class="unit-info">
    <div id="hdr-name">---</div>
    <div id="hdr-mac">---</div>
  </div>
</div>

<div class="tabs">
  <div class="tab active" onclick="showTab('prehled')">Přehled</div>
  <div class="tab" onclick="showTab('senzory')">Senzory</div>
  <div class="tab" onclick="showTab('nastaveni')">Nastavení</div>
  <div class="tab" onclick="showTab('system')">Systém</div>
</div>

<!-- PŘEHLED -->
<div id="tab-prehled" class="content active">
  <div class="refresh-info">Aktualizace každých 10s</div>
  <div class="card">
    <h2>Aktuální teploty</h2>
    <div id="sensor-grid" class="sensor-grid">Načítám...</div>
  </div>
  <div class="card">
    <h2>Stav systému</h2>
    <div id="status-grid" class="status-grid">Načítám...</div>
  </div>
</div>

<!-- SENZORY -->
<div id="tab-senzory" class="content">
  <div class="card">
    <h2>Správa senzorů</h2>
    <p style="font-size:0.85em;color:#aaa;margin-bottom:12px;">
      Senzory bez jména (žlutý okraj) nejsou odesílány do API.
      Po pojmenování se okamžitě synchronizují.
    </p>
    <div id="sensor-manage">Načítám...</div>
  </div>
</div>

<!-- NASTAVENÍ -->
<div id="tab-nastaveni" class="content">
  <div class="card">
    <h2>Konfigurace jednotky</h2>
    <div class="form-group">
      <label>Název jednotky</label>
      <input type="text" id="cfg-name" placeholder="např. Kotelna-brat">
    </div>
    <div class="form-group">
      <label>API URL</label>
      <input type="text" id="cfg-url" placeholder="https://thermo-api.p7d.cz">
    </div>
    <div class="form-group">
      <label>API Token</label>
      <input type="password" id="cfg-token" placeholder="zadej token">
    </div>
    <button class="btn btn-primary" onclick="saveConfig()">Uložit</button>
    <div id="cfg-msg" class="msg"></div>
  </div>

  <div class="card">
    <h2>WiFi připojení</h2>
    <button class="btn btn-secondary" onclick="scanWifi()" style="margin-bottom:12px;">
      Vyhledat sítě
    </button>
    <ul id="wifi-list" class="wifi-list"></ul>
    <div class="form-group" style="margin-top:12px;">
      <label>SSID</label>
      <input type="text" id="wifi-ssid" placeholder="Název sítě">
    </div>
    <div class="form-group">
      <label>Heslo</label>
      <input type="password" id="wifi-pass" placeholder="Heslo">
    </div>
    <button class="btn btn-primary" onclick="saveWifi()">Připojit</button>
    <div id="wifi-msg" class="msg"></div>
  </div>
</div>

<!-- SYSTÉM -->
<div id="tab-system" class="content">
  <div class="card">
    <h2>Informace o systému</h2>
    <div id="sys-info" class="status-grid">Načítám...</div>
  </div>
  <div class="card">
    <h2>Logy (WARN a výše)</h2>
    <ul id="log-list" class="log-list"></ul>
  </div>
</div>

<script>
// ── Tab navigace ──────────────────────────────────────────
function showTab(name) {
  document.querySelectorAll('.tab').forEach((t,i) => {
    t.classList.toggle('active', ['prehled','senzory','nastaveni','system'][i] === name);
  });
  document.querySelectorAll('.content').forEach(c => c.classList.remove('active'));
  document.getElementById('tab-' + name).classList.add('active');
  if (name === 'system') loadLogs();
  if (name === 'senzory') loadSensors();
}

// ── Fetch helpers ─────────────────────────────────────────
async function getJson(url) {
  const r = await fetch(url);
  return r.json();
}

async function postJson(url, data) {
  const r = await fetch(url, {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify(data)
  });
  return r.json();
}

function showMsg(id, ok, text) {
  const el = document.getElementById(id);
  el.className = 'msg ' + (ok ? 'msg-ok' : 'msg-err');
  el.textContent = text;
  el.style.display = 'block';
  setTimeout(() => el.style.display = 'none', 3000);
}

// ── Přehled ───────────────────────────────────────────────
async function loadStatus() {
  try {
    const d = await getJson('/api/status');

    document.getElementById('hdr-name').textContent = d.unit_name || '---';
    document.getElementById('hdr-mac').textContent  = d.unit_mac  || '---';

    // Senzory
    const grid = document.getElementById('sensor-grid');
    if (!d.sensors || d.sensors.length === 0) {
      grid.innerHTML = '<p style="color:#aaa">Žádné senzory nenalezeny</p>';
    } else {
      grid.innerHTML = d.sensors.map(s => `
        <div class="sensor-item ${s.named ? '' : 'sensor-unnamed'}">
          <div class="sensor-name">${s.name || '⚠ Bez jména'}</div>
          <div class="sensor-value ${s.valid ? '' : 'invalid'}">
            ${s.valid ? s.value.toFixed(1) + ' °C' : '---'}
          </div>
          <div class="sensor-rom">${s.rom_id}</div>
        </div>
      `).join('');
    }

    // Status
    const sg = document.getElementById('status-grid');
    sg.innerHTML = `
      <div class="status-item">
        <div class="status-label">WiFi</div>
        <div class="status-value ${d.wifi_connected ? 'status-ok' : 'status-err'}">
          ${d.wifi_connected ? d.wifi_ssid : 'Odpojeno'}
        </div>
      </div>
      <div class="status-item">
        <div class="status-label">IP adresa</div>
        <div class="status-value">${d.wifi_ip || '---'}</div>
      </div>
      <div class="status-item">
        <div class="status-label">API</div>
        <div class="status-value">${d.api_url || 'Nenastaveno'}</div>
      </div>
      <div class="status-item">
        <div class="status-label">Uptime</div>
        <div class="status-value">${formatUptime(d.uptime_s)}</div>
      </div>
    `;
  } catch(e) {
    console.error('Status load failed:', e);
  }
}

function formatUptime(s) {
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  return h + 'h ' + m + 'm';
}

// ── Senzory ───────────────────────────────────────────────
async function loadSensors() {
  const d = await getJson('/api/sensors');
  const el = document.getElementById('sensor-manage');
  if (!d.sensors || d.sensors.length === 0) {
    el.innerHTML = '<p style="color:#aaa">Žádné senzory</p>';
    return;
  }
  el.innerHTML = d.sensors.map(s => `
    <div class="sensor-item ${s.named ? '' : 'sensor-unnamed'}" style="margin-bottom:8px;">
      <div class="sensor-name">${s.named ? s.name : '⚠ Bez jména'}</div>
      <div class="sensor-rom">${s.rom_id}</div>
      <div class="sensor-value ${s.valid ? '' : 'invalid'}" style="font-size:1.2em;">
        ${s.valid ? s.value.toFixed(1) + ' °C' : '---'}
      </div>
      <div class="sensor-rename">
        <input type="text" id="name-${s.rom_id}"
               placeholder="Zadej jméno" value="${s.name || ''}">
        <button class="btn btn-primary"
                onclick="renameSensor('${s.rom_id}')">Uložit</button>
      </div>
    </div>
  `).join('');
}

async function renameSensor(romId) {
  const name = document.getElementById('name-' + romId).value.trim();
  if (!name) return;
  const r = await postJson('/api/sensors', {rom_id: romId, name: name});
  if (r.ok) {
    loadSensors();
    loadStatus();
  }
}

// ── Nastavení ─────────────────────────────────────────────
async function loadConfig() {
  const d = await getJson('/api/config');
  document.getElementById('cfg-name').value = d.unit_name || '';
  document.getElementById('cfg-url').value  = d.api_url   || '';
}

async function saveConfig() {
  const data = {
    unit_name: document.getElementById('cfg-name').value.trim(),
    api_url:   document.getElementById('cfg-url').value.trim(),
    api_token: document.getElementById('cfg-token').value.trim(),
  };
  if (!data.unit_name) { showMsg('cfg-msg', false, 'Název je povinný'); return; }
  const r = await postJson('/api/config', data);
  showMsg('cfg-msg', r.ok, r.ok ? 'Uloženo' : 'Chyba uložení');
}

async function scanWifi() {
  const list = document.getElementById('wifi-list');
  list.innerHTML = '<li style="color:#aaa;padding:8px">Vyhledávám...</li>';
  const d = await getJson('/api/scan');
  list.innerHTML = d.networks.map(n => `
    <li class="wifi-item" onclick="selectWifi('${n.ssid}')">
      <span>${n.ssid} ${n.secure ? '🔒' : ''}</span>
      <span class="wifi-rssi">${n.rssi} dBm</span>
    </li>
  `).join('');
}

function selectWifi(ssid) {
  document.getElementById('wifi-ssid').value = ssid;
  document.getElementById('wifi-pass').focus();
}

async function saveWifi() {
  const data = {
    ssid: document.getElementById('wifi-ssid').value.trim(),
    pass: document.getElementById('wifi-pass').value,
  };
  if (!data.ssid) { showMsg('wifi-msg', false, 'SSID je povinné'); return; }
  const r = await postJson('/api/wifi', data);
  showMsg('wifi-msg', r.ok, r.ok ? 'Uloženo, připojuji...' : 'Chyba');
}

// ── Systém + logy ─────────────────────────────────────────
async function loadLogs() {
  const d = await getJson('/api/logs');
  const list = document.getElementById('log-list');
  if (!d.logs || d.logs.length === 0) {
    list.innerHTML = '<li style="color:#aaa;padding:8px">Žádné záznamy</li>';
    return;
  }
  list.innerHTML = d.logs.map(l => `
    <li class="log-item log-${l.level}">
      [${l.level}] ${l.module}: ${l.message}
      <span style="color:#555;float:right">${formatUptime(l.uptime)}</span>
    </li>
  `).join('');

  // Sys info
  const si = document.getElementById('sys-info');
  if (si) {
    getJson('/api/status').then(d => {
      si.innerHTML = `
        <div class="status-item">
          <div class="status-label">Firmware</div>
          <div class="status-value">${d.fw_version}</div>
        </div>
        <div class="status-item">
          <div class="status-label">MAC</div>
          <div class="status-value">${d.unit_mac}</div>
        </div>
        <div class="status-item">
          <div class="status-label">Uptime</div>
          <div class="status-value">${formatUptime(d.uptime_s)}</div>
        </div>
        <div class="status-item">
          <div class="status-label">IP</div>
          <div class="status-value">${d.wifi_ip || '---'}</div>
        </div>
      `;
    });
  }
}

// ── Auto refresh ──────────────────────────────────────────
loadStatus();
loadConfig();
setInterval(loadStatus, 10000);
</script>
</body>
</html>
)=====";

const char* getWebUI() {
    return WEB_UI;
}