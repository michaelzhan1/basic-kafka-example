CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TABLE logs (
    ts TIMESTAMPTZ NOT NULL,
    level TEXT NOT NULL,
    message TEXT NOT NULL
);

SELECT create_hypertable('logs', 'ts');
