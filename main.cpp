#include "MotorController.h"
#include <iostream>
#include <unistd.h>

int main() {
    MotorController motor("./LMD_P42.toml", "192.168.33.1");

    if (!motor.connect()) {
        std::cerr << "Application exited due to failed connection" << std::endl;
        return 1;
    }
    
    int32_t pos_1;
    int32_t pos_2 = 0;

    std::cout << "Enter rotational amount: ";
    std::cin >> pos_1;

    if (motor.setAbsolutePosition(pos_1)) {
        std::cout << "Successfully commanded move to " << pos_1 << std::endl << "Waiting for move to complete..." << std::endl;
        std::cout << "Moving flag: " << motor.isMoving() << std::endl;
        sleep(5);
        std::cout << "Moving flag: " << motor.isMoving() << std::endl;
    }

    if (motor.setAbsolutePosition(pos_2)) {
        std::cout << "Successfully commanded move to " << pos_2 << std::endl << "Waiting for move to complete..." << std::endl;
        std::cout << "Moving flag: " << motor.isMoving() << std::endl;
        sleep(5);
        std::cout << "Moving flag: " << motor.isMoving() << std::endl;
    }

    std::cout << "Motor controll sequence finished" << std::endl;
    return 0;
}