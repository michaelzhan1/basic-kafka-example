CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TABLE logs (
    ts TIMESTAMPTZ NOT NULL,
    level TEXT NOT NULL,
    message TEXT NOT NULL
);

SELECT create_hypertable('logs', 'ts');

INSERT INTO logs (ts, level, message) VALUES
    (NOW(), 'INFO', 'This is an info log message on startup.'),
    (NOW(), 'ERROR', 'This is an error log message on startup.'),
    (NOW(), 'DEBUG', 'This is a debug log message on startup.');