# compiler
CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wunused-variable -Werror -Iinclude -Isrc/common

AR := ar
ARFLAGS := rcs

# libraries
KAFKA_LIBS := -lrdkafka++ -lrdkafka
DB_LIBS := -lpqxx -lpq
THREAD_LIBS := -pthread

# directories
SRC := ./src
BUILD := ./build
BIN := $(BUILD)/bin
OBJ := $(BUILD)/obj
LIB := $(BUILD)/lib

# executables
PRODUCER := $(BIN)/producer
CONSUMER := $(BIN)/consumer
SERVER := $(BIN)/server

# sources
COMMON_SRCS := $(wildcard $(SRC)/common/*.cpp)
DB_SRC := $(SRC)/common/database.cpp
COMMON_CORE_SRCS := $(filter-out $(DB_SRC),$(COMMON_SRCS))

PRODUCER_SRCS := $(wildcard $(SRC)/producer/*.cpp)
CONSUMER_SRCS := $(wildcard $(SRC)/consumer/*.cpp)
SERVER_SRCS := $(shell find $(SRC)/server -name '*.cpp' | sort)

# object files
COMMON_CORE_OBJS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(COMMON_CORE_SRCS))
DB_OBJS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(DB_SRC))

PRODUCER_OBJS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(PRODUCER_SRCS))
CONSUMER_OBJS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(CONSUMER_SRCS))
SERVER_OBJS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SERVER_SRCS))

$(SERVER_OBJS): CXXFLAGS += -Isrc/server

# static libs
LIBCOMMON := $(LIB)/libcommon.a
LIBDB := $(LIB)/libdatabase.a

.PHONY: all clean producer consumer server

# targets
all: producer consumer server

# libraries
$(LIBCOMMON): $(COMMON_CORE_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

$(LIBDB): $(DB_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

# linking executables
producer: $(PRODUCER)
consumer: $(CONSUMER)
server: $(SERVER)

$(PRODUCER): $(PRODUCER_OBJS) $(LIBCOMMON)
	@mkdir -p $(BIN)
	$(CXX) $^ -o $@ $(KAFKA_LIBS) $(THREAD_LIBS)

$(CONSUMER): $(CONSUMER_OBJS) $(LIBCOMMON) $(LIBDB)
	@mkdir -p $(BIN)
	$(CXX) $^ -o $@ $(KAFKA_LIBS) $(DB_LIBS)

$(SERVER): $(SERVER_OBJS) $(LIBCOMMON)
	@mkdir -p $(BIN)
	$(CXX) $^ -o $@

# compile rule
$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean
clean:
	rm -rf $(BUILD)

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
