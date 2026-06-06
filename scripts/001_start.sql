CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TYPE status_type AS ENUM ('OK', 'WARN', 'ERROR');

CREATE TABLE logs (
    ts TIMESTAMPTZ NOT NULL,
    worker_id TEXT NOT NULL,
    status status_type NOT NULL
);

SELECT create_hypertable('logs', 'ts');

INSERT INTO logs (ts, worker_id, status) VALUES
    (NOW(), 'worker 1', 'OK'),
    (NOW(), 'worker 2', 'ERROR'),
    (NOW(), 'worker 3', 'WARN');