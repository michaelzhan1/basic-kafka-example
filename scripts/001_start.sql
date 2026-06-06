CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TYPE status_type AS ENUM ('OK', 'WARN', 'ERROR');

CREATE TABLE logs (
    ts TIMESTAMPTZ NOT NULL,
    worker_id TEXT NOT NULL,
    status status_type NOT NULL
);

SELECT create_hypertable('logs', 'ts');
SELECT add_retention_policy('logs', INTERVAL '5 minutes');