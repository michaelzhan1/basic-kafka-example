# compiler and flags
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude -Isrc/common -O2
KAFKA_LDFLAGS := -lrdkafka++ -lrdkafka
DB_LDFLAGS := -lpqxx -lpq
THREAD_LDFLAGS := -lpthread

# directories
SRC_DIR   := ./src
BUILD_DIR := ./build
BIN_DIR   := $(BUILD_DIR)/bin
OBJ_DIR   := $(BUILD_DIR)/obj

# executable
CONSUMER_EXE := $(BIN_DIR)/consumer
PRODUCER_EXE := $(BIN_DIR)/producer
SERVER_EXE := $(BIN_DIR)/server

# find all cpp files
COMMON_SRCS := $(wildcard $(SRC_DIR)/common/*.cpp)
CONSUMER_SRCS := $(wildcard $(SRC_DIR)/consumer/*.cpp)
PRODUCER_SRCS := $(wildcard $(SRC_DIR)/producer/*.cpp)
SERVER_SRCS := $(wildcard $(SRC_DIR)/server/*.cpp)

# object files
COMMON_SHARED_SRCS := $(filter-out $(SRC_DIR)/common/database.cpp, $(COMMON_SRCS))
COMMON_DB_SRCS := $(SRC_DIR)/common/database.cpp

COMMON_SHARED_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_SHARED_SRCS))
COMMON_DB_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_DB_SRCS))
CONSUMER_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CONSUMER_SRCS)) $(COMMON_SHARED_OBJS) $(COMMON_DB_OBJS)
PRODUCER_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(PRODUCER_SRCS)) $(COMMON_SHARED_OBJS)
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

.PHONY: all clean consumer producer server

all: consumer producer server

consumer: $(CONSUMER_EXE)
producer: $(PRODUCER_EXE)
server: $(SERVER_EXE)

# linking executables
$(CONSUMER_EXE): $(CONSUMER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(KAFKA_LDFLAGS) $(DB_LDFLAGS)

$(PRODUCER_EXE): $(PRODUCER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(KAFKA_LDFLAGS) $(THREAD_LDFLAGS)

$(SERVER_EXE): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# docker stuff
docker-producer-build:
	docker compose build producer

docker-consumer-build:
	docker compose build consumer

docker-server-build:
	docker compose build server

docker-producer:
	docker compose -p log-handler up producer

docker-consumer:
	docker compose -p log-handler up --scale consumer=4 consumer

docker-server:
	docker compose -p log-handler up server
