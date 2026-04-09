#pragma once

#include <gpiod.hpp>
#include <string>

class LimitSwitch {
public:
    LimitSwitch(unsigned int pin, const std::string &chip_path);
    bool get();

private:
    gpiod::chip _chip;
    unsigned int _pin;
    gpiod::line_request _request;
};