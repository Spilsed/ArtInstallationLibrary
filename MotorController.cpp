#include "MotorController.h"
#include <iostream>
#include <errno.h>

MotorController::MotorController(const std::string &ip_address, int port, int slave_id) {
    ip_address_ = ip_address;
    port_ = port;
    slave_id_ = slave_id;
}

MotorController::~MotorController() {
    if (ctx_) {
        modbus_close(ctx_);
        modbus_free(ctx_);

        ctx_ = nullptr;
        std::cout << "[INFO] Modbux Connection Closed" << std::endl;
    }
}

bool MotorController::connect() {
    ctx_ = modbus_new_tcp(ip_address_.c_str(), port_);

    if (!ctx_) {
        logError("Failed to create Modbus context");
        return false;
    }

    modbus_set_slave(ctx_, slave_id_);

    std::cout << "[INFO] Attemtping to connect to " << ip_address_ << ":" << port_ << std::endl;

    if (modbus_connect(ctx_) == -1) {
        logError("Modbus connection failed");
        modbus_free(ctx_);
        ctx_ = nullptr;
        return false;
    }

    std::cout << "[INFO] Successfully connected to motor" << std::endl;
    return true;
}

bool MotorController::readFlag(int address, bool &value) {
    if (!ctx_) {
        logError("Cannot read flag: Not connected");
        return false;
    }

    uint8_t status[1];
    int rc = modbus_read_input_bits(ctx_, address, 1, status);

    if (rc == -1) {
        logError("Failed to read flag status");
        return false;
    }

    value = (status[0] == 1);
    return true;
}

bool MotorController::read8BitRegister(int address, int8_t &value) {
    if (!ctx_) {
        logError("Cannot read: Not connected");
        return false;
    }

    uint8_t data[1];
    int rc = modbus_read_bits(ctx_, address, 1, data);

    if (rc == -1) {
        logError("Failed to read 8-bit register");
        return false;
    }

    value = (int8_t)data[0];

    return true;
}

bool MotorController::write8BitRegister(int address, int8_t value) {
    if (!ctx_) {
        logError("Cannot write: Not connected");
        return false;
    }

    uint8_t data[1];
    data[0] = (uint8_t)value;

    int rc = modbus_write_bits(ctx_, address, 1, data);

    if (rc == -1) {
        logError("Failed to write 8-bit register");
        return false;
    }

    return true;
}

bool MotorController::read32BitRegister(int address, int32_t &value) {
    if (!ctx_) {
        logError("Cannot read: Not connected");
        return false;
    }

    uint16_t data[2];
    int rc = modbus_read_registers(ctx_, address, 2, data);

    if (rc == -1) {
        logError("Failed to read 32-bit registers");
        return false;
    }

    value = (int32_t)(((uint32_t)data[0] << 16) | data[1]);

    return true;
}

bool MotorController::write32BitRegister(int address, int32_t value) {
    if (!ctx_) {
        logError("Cannot write: Not connected");
        return false;
    }

    uint16_t data[2];

    // Split 32-bit integer into two 16-bit values (big endian)
    data[0] = (uint16_t)(value >> 16);
    data[1] = (uint16_t)value;

    int rc = modbus_write_registers(ctx_, address, 2, data);

    if (rc == -1) {
        logError("Failed to write 32-bit registers");
        return false;
    }

    return true;
}

bool MotorController::isMoving() {
    if (!ctx_) {
        logError("Cannot check status: Not connected");
        return false;
    }

    uint8_t status[1];

    int rc = modbus_read_input_bits(ctx_, MOVING_FLAG_ADDRESS, 1, status);

    if (rc == -1) {
        logError("Failed to read Busy Flag status");
        return false;
    }

    return (status[0] == 1);
}

int32_t MotorController::getCurrentPosition() {
    int32_t current_position;

    if (read32BitRegister(ABS_POSITION_REGISTER_START, current_position)) {
        return current_position;
    } else {
        return 0;
    }
}

int32_t MotorController::getCurrentVelocity() {
    int32_t current_velocity;

    if (read32BitRegister(READ_AXIS_VELOCITY_START, current_velocity)) {
        return current_velocity;
    } else {
        return 0;
    }
}

int8_t MotorController::getCurrentMicrostepResolution() {
    int8_t current_microstep_resolution;

    if (read8BitRegister(MICROSTEP_RESOLUTION_ADDRESS, current_microstep_resolution)) {
        return current_microstep_resolution;
    } else {
        return 0;
    }
}

int32_t MotorController::getInitialVelocity() {
    int32_t initial_velocity;

    if (read32BitRegister(INITIAL_VELOCITY_REGISTER_START, initial_velocity)) {
        return initial_velocity;
    } else {
        return 0;
    }
}

int32_t MotorController::getMaxVelocity() {
    int32_t max_velocity;

    if (read32BitRegister(MAX_VELOCITY_REGISTER_START, max_velocity)) {
        return max_velocity;
    } else {
        return 0;
    }
}

bool MotorController::setMicrostepResolution(int8_t microstep_resolution) {
    std::cout << "[INFO] Setting microstep resolution to: " << microstep_resolution << std::endl;
    return write8BitRegister(MICROSTEP_RESOLUTION_ADDRESS, microstep_resolution);
}

bool MotorController::setAbsolutePosition(int32_t target_position) {
    std::cout << "[INFO] Setting target position to: " << target_position << std::endl;
    return write32BitRegister(ABS_POSITION_REGISTER_START, target_position);
}

bool MotorController::saveSettings() {
    if (!ctx_) {
        logError("Cannot save settings: Not connected");
        return false;
    }

    std::cout << "[INFO] Saving all parameters" << std::endl;

    int rc = modbus_write_register(ctx_, SAVE_SETTINGS_REGISTER, 1);

    if (rc == -1) {
        logError("Failed to write to Save Settings register");
        return false;
    }

    std::cout << "[INFO] Save command successfuly" << std::endl;

    return true;
}

bool MotorController::setInitialVelocity(int32_t initial_velocity) {
    std::cout << "[INFO] Setting initial velocity to: " << initial_velocity << std::endl;
    return write32BitRegister(INITIAL_VELOCITY_REGISTER_START, initial_velocity);
}

bool MotorController::setMaxVelocity(int32_t max_velocity) {
    std::cout << "[INFO] Setting initial velocity to: " << max_velocity << std::endl;
    return write32BitRegister(MAX_VELOCITY_REGISTER_START, max_velocity);
}

void MotorController::logError(const std::string &message) const {
    if (ctx_) {
        std::cerr << "[ERROR] " << message << ": " << modbus_strerror(errno) << std::endl;
    } else {
        std::cerr << "[ERROR] " << message << std::endl;
    }
}