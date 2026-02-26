CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -pedantic -g
LDLIBS := -lmodbus -lgpiodcxx

BUILD_DIR := build

SRCS := main.cpp MotorController.cpp StepperController.cpp

HDRS := MotorController.h StepperController.h

OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET := $(BUILD_DIR)/motor_controller.out

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp $(HDRS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run