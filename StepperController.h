#ifndef STEPPER_CONTROLLER_H
#define STEPPER_CONTROLLER_H

#include <gpiod.hpp>
#include <string>

class StepperController {
public:
    StepperController(unsigned int step_pin, unsigned int dir_pin, const std::string &chip_path);
    void move(int steps, bool clockwise, int delay_us);

private:
    gpiod::chip _chip;
    unsigned int _step_offset;
    unsigned int _dir_offset;
    gpiod::line_request _request;
};

#endif