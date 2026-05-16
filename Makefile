# compiler
CXX := g++

# compiler flags
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude -O2

# directories
BUILD_DIR := ./build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj
SRC_DIR := ./src

# executable
TARGET := $(BIN_DIR)/app

# find all cpp files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

#object files
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# default target
all: $(TARGET)

# link object files to create executable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run:
	$(TARGET)

# clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

