#include "MotorController.h"
#include <iostream>
#include <sstream> 
#include <fstream>
#include <unordered_map>
#include <functional>
#include <errno.h>

MotorController::MotorController(const std::string &profile_path, const std::string &ip_address, int port, int slave_id) {
    ip_address_ = ip_address;
    port_ = port;
    slave_id_ = slave_id;
    
    loadProfile(profile_path);
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

bool MotorController::loadProfile(const std::string &profile_path) {
    std::ifstream file(profile_path);

    if (!file.is_open()) {
        logError("Couldn't open the profile file");
        return false;
    }

    std::string text;
    std::string current_table = "";
    std::string current_key = "";
    while (file >> text) {
        if (text[0] == '[') {
            current_table = text;
            continue;
        }

        if (current_key == "") {
            current_key = text;
            continue;
        }

        if (text[0] == '=') {
            file >> text;
        }

        static const std::unordered_map<std::string, std::function<void(const std::string&)>> register_key_handlers = {
            {"read-axis-velocity", [&](const std::string& val_text) {
                READ_AXIS_VELOCITY_START = std::stoul(val_text, nullptr, 16);
            }},
            {"microstep-resolution", [&](const std::string& val_text) {
                MICROSTEP_RESOLUTION_ADDRESS = std::stoul(val_text, nullptr, 16);
            }},
            {"moving-flag", [&](const std::string& val_text) {
                MOVING_FLAG_ADDRESS = std::stoul(val_text, nullptr, 16);
            }},
            {"position", [&](const std::string& val_text) {
                ABS_POSITION_REGISTER_START = std::stoul(val_text, nullptr, 16);
            }},
            {"save-settings", [&](const std::string& val_text) {
                SAVE_SETTINGS_REGISTER = std::stoul(val_text, nullptr, 16);
            }},
            {"initial-velocity", [&](const std::string& val_text) {
                INITIAL_VELOCITY_REGISTER_START = std::stoul(val_text, nullptr, 16);
            }},
            {"max-velocity", [&](const std::string& val_text) {
                MAX_VELOCITY_REGISTER_START = std::stoul(val_text, nullptr, 16);
            }}
        };

        std::unordered_map<std::string, std::function<void(const std::string&)>> handlers;
        if (current_table == "[registers]") {
            handlers = register_key_handlers;
        } else {
            continue;
        }

        auto it = handlers.find(current_key);
        if (it != handlers.end()) {
            it->second(text);
        } else {
            std::cerr << "[Warning] Unknown configuration key: " << current_key << std::endl;
        }

        current_key = "";
    }

    return true;
}

bool MotorController::readFlag(int address, bool &value) const {
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

bool MotorController::read8BitRegister(int address, int8_t &value) const {
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

bool MotorController::read32BitRegister(int address, int32_t &value) const {
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
    bool flag;
    
    readFlag(MOVING_FLAG_ADDRESS, flag);

    return flag;
}

int32_t MotorController::getCurrentPosition() const {
    int32_t current_position;

    if (read32BitRegister(ABS_POSITION_REGISTER_START, current_position)) {
        return current_position;
    } else {
        return 0;
    }
}

int32_t MotorController::getCurrentVelocity() const {
    int32_t current_velocity;

    if (read32BitRegister(READ_AXIS_VELOCITY_START, current_velocity)) {
        return current_velocity;
    } else {
        return 0;
    }
}

int8_t MotorController::getCurrentMicrostepResolution() const {
    int8_t current_microstep_resolution;

    if (read8BitRegister(MICROSTEP_RESOLUTION_ADDRESS, current_microstep_resolution)) {
        return current_microstep_resolution;
    } else {
        return 0;
    }
}

int32_t MotorController::getInitialVelocity() const {
    int32_t initial_velocity;

    if (read32BitRegister(INITIAL_VELOCITY_REGISTER_START, initial_velocity)) {
        return initial_velocity;
    } else {
        return 0;
    }
}

int32_t MotorController::getMaxVelocity() const {
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
    std::stringstream stream;

    if (ctx_) {
        stream << message << ": " << modbus_strerror(errno) << std::endl;
    } else {
        stream << message << std::endl;
    }

    logError(stream);
}

void MotorController::logError(std::stringstream &message_stream) const {
    std::string stream_string;
    message_stream >> stream_string;

    std::cerr << "[ERROR] " << stream_string;

    if (ctx_) {
        std::cerr << ": " << modbus_strerror(errno) << std::endl;
    }

    std::cerr << std::endl;
}