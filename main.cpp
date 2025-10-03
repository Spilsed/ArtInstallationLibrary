#include "MotorController.h"
#include <iostream>
#include <unistd.h>

int main() {
    MotorController motor("192.168.33.1");

    if (!motor.connect()) {
        std::cerr << "Application exited due to failed connection" << std::endl;
        return 1;
    }

    int32_t pos_1 = 1000;
    int32_t pos_2 = 0;

    if (motor.setAbsolutePosition(pos_1)) {
        std::cout << "Successfully commanded move to " << pos_1 << std::endl << "Waiting for move to complete..." << std::endl;
        sleep(5);
    }

    if (motor.setAbsolutePosition(pos_2)) {
        std::cout << "Successfully commanded move to " << pos_2 << std::endl << "Waiting for move to complete..." << std::endl;
        sleep(5);
    }

    std::cout << "Motor controll sequence finished" << std::endl;
    return 0;
}