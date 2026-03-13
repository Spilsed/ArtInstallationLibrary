#pragma once

#include <gpiod.hpp>
#include <string>

class StepperController {
    public:
        StepperController(
            unsigned int step_pin,
            unsigned int dir_pin,
            unsigned int enable_pin,
            unsigned int microstep_pins[4],
            const std::string &chip_path
        );
        void move(int steps, bool clockwise, int delay_us);
        void setMicrostep(short value);
        void setEnabled(bool value);

        bool isEnabled();

    private:
        gpiod::chip _chip;
        unsigned int _step_offset;
        unsigned int _dir_offset;
        unsigned int _enable_offset;
        unsigned int *_microstep_offsets;
        gpiod::line_request _request;

        bool _enabled;
};