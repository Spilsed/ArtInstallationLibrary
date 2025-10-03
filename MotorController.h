#ifndef ___MOTOR_CONTROLLER
#define ___MOTOR_CONTROLLER

#include <string>
#include <cstdint>
#include <modbus/modbus.h>
#include <unordered_map>
#include <functional>

class MotorController {
    public:
        MotorController(const std::string &profile_path, const std::string &ip_address, int port = 502, int slave_id = 1);
        ~MotorController();

        bool connect();

        bool isMoving();

        int32_t getCurrentPosition() const;
        int32_t getCurrentVelocity() const;
        int8_t getCurrentMicrostepResolution() const;
        int32_t getInitialVelocity() const;
        int32_t getMaxVelocity() const;

        bool setMicrostepResolution(int8_t microstep_resolution);
        bool setAbsolutePosition(int32_t target_position);
        bool saveSettings();
        bool setInitialVelocity(int32_t inital_velocity);
        bool setMaxVelocity(int32_t max_velocity);
    
    private:
        modbus_t *ctx_ = nullptr;
        std::string ip_address_;
        int port_;
        int slave_id_;

        int READ_AXIS_VELOCITY_START;

        int MICROSTEP_RESOLUTION_ADDRESS;
        int MOVING_FLAG_ADDRESS;
        int ABS_POSITION_REGISTER_START;
        int SAVE_SETTINGS_REGISTER;
        int INITIAL_VELOCITY_REGISTER_START;
        int MAX_VELOCITY_REGISTER_START;

        int MAX_VELOCITY;

        bool loadProfile(const std::string &profile_path);

        bool readFlag(int address, bool &value) const;
        bool read8BitRegister(int address, int8_t &value) const;
        bool write8BitRegister(int address, int8_t value);
        bool read32BitRegister(int address, int32_t &value) const;
        bool write32BitRegister(int address, int32_t value);

        void logError(const std::string &message) const;
        void logError(std::stringstream &message_stream) const;
};

#endif