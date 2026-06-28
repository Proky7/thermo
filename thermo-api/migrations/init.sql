USE HomeSys;

CREATE TABLE IF NOT EXISTS thermo_units (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    mac         VARCHAR(17) NOT NULL UNIQUE,
    name        VARCHAR(64),
    fw_version  VARCHAR(32),
    last_seen   DATETIME,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS thermo_sensors (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    unit_id     INT NOT NULL,
    rom_id      VARCHAR(32) NOT NULL,
    name        VARCHAR(64),
    last_seen   DATETIME,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (unit_id) REFERENCES thermo_units(id),
    UNIQUE KEY uq_unit_sensor (unit_id, rom_id)
);

CREATE TABLE IF NOT EXISTS thermo_logs (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    unit_id     INT,
    level       ENUM('DEBUG','INFO','WARN','ERROR','CRITICAL'),
    message     TEXT,
    ts          DATETIME NOT NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_unit_ts (unit_id, ts)
);