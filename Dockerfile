FROM ubuntu:24.04 AS base

RUN apt-get update && apt-get install -y \
    build-essential \
    librdkafka-dev \
    nlohmann-json3-dev \
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

# dev
FROM base AS dev
# Install all dependencies for both services + development tools
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository universe \
    && apt-get update \
    && apt-get install -y \
    libpqxx-dev \
    git \
    gdb \
    valgrind \
    && rm -rf /var/lib/apt/lists/*
