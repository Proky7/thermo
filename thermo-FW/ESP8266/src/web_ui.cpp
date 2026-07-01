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
  .header h1 { font-size: 1.4em; color: #4fc3f7; }
  .header .unit-info { font-size: 0.8em; color: #aaa; text-align: right; }

  .tabs {
    display: flex;
    background: #16213e;
    border-bottom: 1px solid #0f3460;
    overflow-x: auto;
  }
  .tab {
    padding: 12px 16px;
    cursor: pointer;
    color: #aaa;
    border-bottom: 2px solid transparent;
    white-space: nowrap;
  }
  .tab.active { color: #4fc3f7; border-bottom-color: #4fc3f7; }

  .content { display: none; padding: 15px; max-width: 600px; margin: 0 auto; }
  .content.active { display: block; }

  .card {
    background: #16213e;
    border-radius: 8px;
    padding: 15px;
    margin-bottom: 12px;
    border: 1px solid #0f3460;
  }
  .card h2 { font-size: 0.95em; color: #4fc3f7; margin-bottom: 10px; }

  .sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
    gap: 8px;
  }
  .sensor-item {
    background: #0f3460;
    border-radius: 6px;
    padding: 10px;
  }
  .sensor-unnamed { border-left: 3px solid #e9a800; }
  .sensor-name { font-size: 0.75em; color: #aaa; margin-bottom: 3px; }
  .sensor-value { font-size: 1.5em; font-weight: bold; color: #4fc3f7; }
  .sensor-value.invalid { color: #555; }
  .sensor-rom { font-size: 0.6em; color: #555; margin-top: 3px; }

  .status-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
  .status-label { font-size: 0.8em; color: #aaa; }
  .status-value { font-size: 0.9em; font-weight: bold; }
  .ok  { color: #4caf50; }
  .err { color: #e94560; }

  .form-group { margin-bottom: 10px; }
  .form-group label { display: block; font-size: 0.8em; color: #aaa; margin-bottom: 3px; }
  .form-group input {
    width: 100%;
    padding: 7px 10px;
    background: #0f3460;
    border: 1px solid #1a4a8a;
    border-radius: 4px;
    color: #eee;
    font-size: 0.85em;
  }

  .btn {
    padding: 7px 16px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-size: 0.85em;
  }
  .btn-primary   { background: #4fc3f7; color: #1a1a2e; font-weight: bold; }
  .btn-secondary { background: #0f3460; color: #eee; }

  .sensor-manage-item {
    background: #0f3460;
    border-radius: 6px;
    padding: 10px;
    margin-bottom: 8px;
  }
  .sensor-manage-item.unnamed { border-left: 3px solid #e9a800; }
  .sensor-rename {
    display: flex;
    gap: 6px;
    margin-top: 6px;
  }
  .sensor-rename input {
    flex: 1;
    padding: 5px 8px;
    background: #1a1a2e;
    border: 1px solid #1a4a8a;
    border-radius: 4px;
    color: #eee;
    font-size: 0.8em;
  }

  .wifi-item {
    padding: 8px 10px;
    background: #0f3460;
    border-radius: 4px;
    margin-bottom: 5px;
    cursor: pointer;
    display: flex;
    justify-content: space-between;
    font-size: 0.85em;
  }
  .wifi-item:hover { background: #1a4a8a; }
  .wifi-rssi { color: #aaa; font-size: 0.8em; }

  .log-item {
    padding: 5px 8px;
    border-radius: 3px;
    margin-bottom: 4px;
    font-size: 0.75em;
    font-family: monospace;
  }
  .log-WARN     { background: #3a2a00; border-left: 3px solid #e9a800; }
  .log-ERROR    { background: #3a0000; border-left: 3px solid #e94560; }
  .log-CRITICAL { background: #5a0000; border-left: 3px solid #ff0000; }

  .msg {
    padding: 7px 10px;
    border-radius: 4px;
    margin-top: 8px;
    font-size: 0.8em;
    display: none;
  }
  .msg-ok  { background: #1a3a1a; color: #4caf50; }
  .msg-err { background: #3a1a1a; color: #e94560; }

  .refresh { font-size: 0.7em; color: #555; text-align: right; margin-bottom: 8px; }
</style>
</head>
<body>

<div class="header">
  <h1>🌡 THERMO</h1>
  <div class="unit-info">
    <div id="hdr-name">---</div>
    <div id="hdr-mac" style="font-size:0.85em;color:#555">---</div>
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
  <div class="refresh">Aktualizace každých 10s</div>
  <div class="card">
    <h2>Aktuální teploty</h2>
    <div id="sensor-grid" class="sensor-grid">Načítám...</div>
  </div>
  <div class="card">
    <h2>Stav</h2>
    <div id="status-grid" class="status-grid">Načítám...</div>
  </div>
</div>

<!-- SENZORY -->
<div id="tab-senzory" class="content">
  <div class="card">
    <h2>Správa senzorů</h2>
    <p style="font-size:0.8em;color:#aaa;margin-bottom:10px;">
      Senzory bez jména (žlutý okraj) nejsou odesílány do API.
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
      <input type="text" id="cfg-name" placeholder="např. Desstovka">
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
    <button class="btn btn-secondary"
            onclick="scanWifi()"
            style="margin-bottom:10px;">
      Vyhledat sítě
    </button>
    <div id="wifi-list"></div>
    <div class="form-group" style="margin-top:10px;">
      <label>SSID</label>
      <input type="text" id="wifi-ssid" placeholder="Název sítě">
    </div>
    <div class="form-group">
      <label>Heslo</label>
      <input type="password" id="wifi-pass" placeholder="Heslo WiFi">
    </div>
    <button class="btn btn-primary" onclick="saveWifi()">Připojit</button>
    <div id="wifi-msg" class="msg"></div>
  </div>
</div>

<!-- SYSTÉM -->
<div id="tab-system" class="content">
  <div class="card">
    <h2>Informace</h2>
    <div id="sys-info" class="status-grid">Načítám...</div>
  </div>
  <div class="card">
    <h2>Logy (WARN+)</h2>
    <div id="log-list">Načítám...</div>
  </div>
</div>

<script>
// ── Navigace ──────────────────────────────────────────────
function showTab(name) {
  const tabs = ['prehled','senzory','nastaveni','system'];
  document.querySelectorAll('.tab').forEach((t,i) => {
    t.classList.toggle('active', tabs[i] === name);
  });
  document.querySelectorAll('.content').forEach(c => {
    c.classList.remove('active');
  });
  document.getElementById('tab-' + name).classList.add('active');
  if (name === 'senzory') loadSensors();
  if (name === 'system')  loadSystem();
}

// ── Helpers ───────────────────────────────────────────────
async function getJson(url) {
  const r = await fetch(url);
  return r.json();
}

async function postJson(url, data) {
  const r = await fetch(url, {
    method: 'POST',
    headers: {'Content-Type':'application/json'},
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

function formatUptime(s) {
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  return h + 'h ' + m + 'm';
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
      grid.innerHTML = '<p style="color:#aaa;font-size:0.85em">Žádné senzory</p>';
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
    document.getElementById('status-grid').innerHTML = `
      <div>
        <div class="status-label">WiFi</div>
        <div class="status-value ${d.wifi_connected ? 'ok' : 'err'}">
          ${d.wifi_connected ? d.wifi_ssid : 'Odpojeno'}
        </div>
      </div>
      <div>
        <div class="status-label">IP</div>
        <div class="status-value">${d.wifi_ip || '---'}</div>
      </div>
      <div>
        <div class="status-label">API</div>
        <div class="status-value" style="font-size:0.75em;word-break:break-all">
          ${d.api_url || 'Nenastaveno'}
        </div>
      </div>
      <div>
        <div class="status-label">Uptime</div>
        <div class="status-value">${formatUptime(d.uptime_s)}</div>
      </div>
    `;
  } catch(e) {
    console.error('Status error:', e);
  }
}

// ── Senzory ───────────────────────────────────────────────
async function loadSensors() {
  const d = await getJson('/api/sensors');
  const el = document.getElementById('sensor-manage');
  if (!d.sensors || d.sensors.length === 0) {
    el.innerHTML = '<p style="color:#aaa;font-size:0.85em">Žádné senzory</p>';
    return;
  }
  el.innerHTML = d.sensors.map(s => `
    <div class="sensor-manage-item ${s.named ? '' : 'unnamed'}">
      <div style="font-size:0.8em;color:#aaa">${s.rom_id}</div>
      <div style="font-size:0.85em;margin:3px 0">
        ${s.named ? s.name : '<span style="color:#e9a800">⚠ Bez jména</span>'}
        ${s.valid
          ? '<span style="float:right;color:#4fc3f7">'
            + s.value.toFixed(1) + ' °C</span>'
          : '<span style="float:right;color:#555">---</span>'}
      </div>
      <div class="sensor-rename">
        <input type="text"
               id="n-${s.rom_id}"
               placeholder="Zadej jméno"
               value="${s.name || ''}">
        <button class="btn btn-primary"
                onclick="renameSensor('${s.rom_id}')">
          Uložit
        </button>
      </div>
    </div>
  `).join('');
}

async function renameSensor(romId) {
  const name = document.getElementById('n-' + romId).value.trim();
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
  if (!data.unit_name) {
    showMsg('cfg-msg', false, 'Název je povinný');
    return;
  }
  const r = await postJson('/api/config', data);
  showMsg('cfg-msg', r.ok, r.ok ? 'Uloženo' : 'Chyba');
}

async function scanWifi() {
  const list = document.getElementById('wifi-list');
  list.innerHTML = '<p style="color:#aaa;font-size:0.85em;padding:8px">Vyhledávám...</p>';
  const d = await getJson('/api/scan');
  list.innerHTML = d.networks.map(n => `
    <div class="wifi-item" onclick="selectWifi('${n.ssid}')">
      <span>${n.ssid} ${n.secure ? '🔒' : ''}</span>
      <span class="wifi-rssi">${n.rssi} dBm</span>
    </div>
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
  if (!data.ssid) {
    showMsg('wifi-msg', false, 'SSID je povinné');
    return;
  }
  const r = await postJson('/api/wifi', data);
  showMsg('wifi-msg', r.ok, r.ok ? 'Uloženo, připojuji...' : 'Chyba');
}

// ── Systém ────────────────────────────────────────────────
async function loadSystem() {
  const d = await getJson('/api/status');
  document.getElementById('sys-info').innerHTML = `
    <div>
      <div class="status-label">Firmware</div>
      <div class="status-value">${d.fw_version}</div>
    </div>
    <div>
      <div class="status-label">MAC</div>
      <div class="status-value" style="font-size:0.8em">${d.unit_mac}</div>
    </div>
    <div>
      <div class="status-label">Uptime</div>
      <div class="status-value">${formatUptime(d.uptime_s)}</div>
    </div>
    <div>
      <div class="status-label">IP</div>
      <div class="status-value">${d.wifi_ip || '---'}</div>
    </div>
  `;

  const logs = await getJson('/api/logs');
  const logEl = document.getElementById('log-list');
  if (!logs.logs || logs.logs.length === 0) {
    logEl.innerHTML = '<p style="color:#aaa;font-size:0.8em">Žádné záznamy</p>';
    return;
  }
  logEl.innerHTML = logs.logs.map(l => `
    <div class="log-item log-${l.level}">
      [${l.level}] ${l.module}: ${l.message}
      <span style="color:#555;float:right">${formatUptime(l.uptime)}</span>
    </div>
  `).join('');
}

// ── Start ─────────────────────────────────────────────────
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