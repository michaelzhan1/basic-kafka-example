FROM ubuntu:24.04 AS base

RUN apt-get update && apt-get install -y \
    build-essential \
    librdkafka-dev \
 && rm -rf /var/lib/apt/lists/*

COPY ./ /app
WORKDIR /app

# producer
FROM base AS producer
RUN make producer
CMD ["./build/bin/producer"]

# consumer
FROM base AS consumer
RUN apt-get update && apt-get install -y software-properties-common \
    && add-apt-repository universe \
    && apt-get update \
    && apt-get install -y libpqxx-dev \
    && rm -rf /var/lib/apt/lists/*
RUN make consumer
CMD ["./build/bin/consumer"]