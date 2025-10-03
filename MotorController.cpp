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

    std::cout << "[INFO] Attemtping to connect to " << ip_address_ << ":" << port_ << "..." << std::endl;

    if (modbus_connect(ctx_) == -1) {
        logError("Modbus connection failed");
        modbus_free(ctx_);
        ctx_ = nullptr;
        return false;
    }

    std::cout << "[INFO] Successfully connected to motor" << std::endl;
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

bool MotorController::setAbsolutePosition(int32_t target_position) {
    std::cout << "[INFO] Setting target position to: " << target_position << std::endl;
    return write32BitRegister(ABS_POSITION_REGISTER_START, target_position);
}

void MotorController::logError(const std::string &message) const {
    if (ctx_) {
        std::cerr << "[ERROR]" << message << ": " << modbus_strerror(errno) << std::endl;
    } else {
        std::cerr << "[ERROR]" << message << std::endl;
    }
}