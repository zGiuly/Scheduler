CREATE DATABASE IF NOT EXISTS scheduler;
USE scheduler;

CREATE TABLE IF NOT EXISTS executors (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    project VARCHAR(100) NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    command TEXT,
    cron VARCHAR(45) NOT NULL,
    type VARCHAR(45) DEFAULT 'async',
    oneshot TINYINT(1) DEFAULT 0,
    disabled TINYINT(1) UNSIGNED DEFAULT 0,
    INDEX idx_project (project)
);

CREATE TABLE IF NOT EXISTS executors_logs (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    project VARCHAR(100) NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    type VARCHAR(45) NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    command TEXT,
    response TEXT NOT NULL,
    execution_time BIGINT,
    INDEX idx_project_logs (project)
);
