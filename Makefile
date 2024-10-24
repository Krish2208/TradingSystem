CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -pedantic
LDFLAGS = -lcurl

# Debug flags
DEBUG_FLAGS = -g -DDEBUG

# Directory structure
BUILD_DIR = build
SRC_DIR = .

# Find all cpp files in the current directory
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BUILD_DIR)/trading_system

# Default target
all: prepare $(TARGET)

# Create build directory
prepare:
	@mkdir -p $(BUILD_DIR)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

# Link the target
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run tests
test: $(TARGET)
	./$(TARGET)

.PHONY: all debug clean test prepare