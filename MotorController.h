#ifndef ___MOTOR_CONTROLLER
#define ___MOTOR_CONTROLLER

#include <string>
#include <cstdint>
#include <modbus/modbus.h>

class MotorController {
    public:
        MotorController(const std::string &ip_address, int port = 502, int slave_id = 1);
        ~MotorController();

        bool connect();
        bool isMoving();
        bool setAbsolutePosition(int32_t target_position);
        bool setInitialVelocity(int32_t inital_velocity);
        bool setMaxVelocity(int32_t max_velocity);
    
    private:
        modbus_t *ctx_ = nullptr;
        std::string ip_address_;
        int port_;
        int slave_id_;

        static constexpr int MOVING_FLAG_ADDRESS = 0x004A;
        static constexpr int ABS_POSITION_REGISTER_START = 0x0057;
        static constexpr int INITIAL_VELOCITY_REGISTER_START = 0x0089;
        static constexpr int MAX_VELOCITY_REGISTER_START = 0x008A;

        bool write32BitRegister(int address, int32_t value);
        void logError(const std::string &message) const;
};

#endif