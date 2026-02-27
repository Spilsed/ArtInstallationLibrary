#pragma once

#include <gpiod.hpp>
#include <string>

class MagnetController {
public:
    MagnetController(unsigned int pin, const std::string &chip_path);
    void set(bool value);

    bool getActive() { return _active; };

private:
    gpiod::chip _chip;
    unsigned int _pin;
    gpiod::line_request _request;
    bool _active = false;
};