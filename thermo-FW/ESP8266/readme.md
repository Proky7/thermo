src/
├── main.cpp
├── config.h          ← výchozí hodnoty včetně tokenu a URL
├── config.cpp        ← load/save z LittleFS
├── logger.h/.cpp     ← stejná logika, 10 záznamů WARN+
├── wifi_manager.h/.cpp  ← AP bez hesla + STA
├── sensor_manager.h/.cpp ← DS18B20, bez bufferu
├── api_client.h/.cpp    ← HTTP POST, BearSSL
├── sensor_sync.h/.cpp   ← stahování jmen z API
├── web_server.h/.cpp    ← endpointy
└── web_ui.h/.cpp        ← HTML (zmenšená verze)