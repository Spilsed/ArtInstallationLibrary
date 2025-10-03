CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -pedantic -g
LDFLAGS := -lmodbus

BUILD_DIR := build

SRCS := main.cpp MotorController.cpp

OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET := $(BUILD_DIR)/motor_controller.out

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp MotorController.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run