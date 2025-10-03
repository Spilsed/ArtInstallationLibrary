#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gpiod.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <stdexcept>

class Controller {
private:
    gpiod::chip* chip;
    gpiod::line_request* request;
    
    std::atomic<bool> running;
    std::thread pwm_thread;
    std::atomic<int> pulse_width_us;
    unsigned int gpio_pin;

    const int PWM_PERIOD_US = 20000;

    // Declaration only
    void pwmLoop(); 
    
public:
    // Declarations only
    Controller();
    ~Controller();

    // Prevent copies
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;

    bool initialize(unsigned int pin, const char *chip_path = "/dev/gpiochip4");
    void setSpeed(int speed_percent);
    void stop();
    void cleanup();
};

#endif // CONTROLLER_H
