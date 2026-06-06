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

# server
FROM base AS server
RUN make server
CMD ["./build/bin/server"]

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

ARG USER_ID=1000
ARG GROUP_ID=1000

# Delete the default user that comes with the base image
RUN deluser --remove-home $(grep :1000: /etc/passwd | cut -d: -f1) || true && \
    delgroup $(grep :1000: /etc/group | cut -d: -f1) || true

# Create your user with the IDs you want
RUN groupadd -g ${GROUP_ID} devuser && \
    useradd -u ${USER_ID} -g devuser -s /bin/bash -m devuser

USER devuser
