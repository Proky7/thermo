CREATE DATABASE IF NOT EXISTS thermo CHARACTER SET utf8mb4;
USE thermo;

CREATE TABLE units (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    mac         VARCHAR(17) NOT NULL UNIQUE,  -- "AA:BB:CC:DD:EE:FF"
    name        VARCHAR(64),
    fw_version  VARCHAR(32),
    last_seen   DATETIME,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE sensors (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    unit_id     INT NOT NULL,
    rom_id      VARCHAR(32) NOT NULL,          -- "28FF1234ABCD0000"
    name        VARCHAR(64),
    last_seen   DATETIME,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (unit_id) REFERENCES units(id),
    UNIQUE KEY uq_unit_sensor (unit_id, rom_id)
);

CREATE TABLE logs (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    unit_id     INT,                           -- NULL pokud neznámá jednotka
    level       ENUM('DEBUG','INFO','WARN','ERROR','CRITICAL'),
    message     TEXT,
    ts          DATETIME NOT NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_unit_ts (unit_id, ts)
);