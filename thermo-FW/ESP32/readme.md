Přehled API endpointů:
GET  /api/status    ← stav systému + aktuální teploty
GET  /api/config    ← aktuální konfigurace
POST /api/config    ← uložení name, api_url, api_token
POST /api/wifi      ← uložení WiFi přihlašovacích údajů
GET  /api/sensors   ← seznam senzorů + teploty + jména
POST /api/sensors   ← pojmenování senzoru
GET  /api/scan      ← scan WiFi sítí
GET  /api/logs      ← posledních 15 WARN+ logů