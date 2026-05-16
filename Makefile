# compiler and flags
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude -O2
LDFLAGS := -lrdkafka++ -lrdkafka -lpthread

# directories
SRC_DIR   := ./src
BUILD_DIR := ./build
BIN_DIR   := $(BUILD_DIR)/bin
OBJ_DIR   := $(BUILD_DIR)/obj

# executable
CONSUMER_EXE := $(BIN_DIR)/consumer
PRODUCER_EXE := $(BIN_DIR)/producer

# find all cpp files
CONSUMER_SRCS := $(wildcard $(SRC_DIR)/consumer/*.cpp)
PRODUCER_SRCS := $(wildcard $(SRC_DIR)/producer/*.cpp)

#object files
CONSUMER_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CONSUMER_SRCS))
PRODUCER_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(PRODUCER_SRCS))

.PHONY: all clean consumer producer

all: consumer producer

consumer: $(CONSUMER_EXE)
producer: $(PRODUCER_EXE)

# linking executables
$(CONSUMER_EXE): $(CONSUMER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(PRODUCER_EXE): $(PRODUCER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
