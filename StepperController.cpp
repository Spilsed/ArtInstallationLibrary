#include "StepperController.h"
#include "Utils.h"
#include <thread>
#include <chrono>
#include <math.h>

using namespace std::chrono;

StepperController::StepperController(unsigned int step_pin,
                                     unsigned int dir_pin,
                                     unsigned int enable_pin,
                                     unsigned int microstep_pins[4],
                                     const std::string &chip_path)
    : _chip(chip_path),
      _step_offset(step_pin),
      _dir_offset(dir_pin),
      _enable_offset(enable_pin),
      _microstep_offsets(microstep_pins),

      _request(_chip.prepare_request()
                .set_request_config(gpiod::request_config().set_consumer("stepper_ctrl"))
                .add_line_settings(
                    {step_pin, dir_pin},
                    gpiod::line_settings()
                        .set_direction(gpiod::line::direction::OUTPUT)
                        .set_output_value(gpiod::line::value::INACTIVE)
                )
                .do_request()) 
{}

void StepperController::move(int steps, bool clockwise, int delay_us) {
    if (steps <= 0) return;

    _request.set_value(_dir_offset, clockwise ? gpiod::line::value::ACTIVE 
                                              : gpiod::line::value::INACTIVE);
    
    std::this_thread::sleep_for(microseconds(10));

    auto delay = microseconds(delay_us);

    for (int i = 0; i < steps; ++i) {
        _request.set_value(_step_offset, gpiod::line::value::ACTIVE);
        std::this_thread::sleep_for(delay);
        
        _request.set_value(_step_offset, gpiod::line::value::INACTIVE);
        std::this_thread::sleep_for(delay);

        delay_us = Utils::lerp(delay_us, delay_us, 1);
        delay = microseconds(delay_us);
    }
}

void StepperController::setMicrostep(short value) {
    value = value / 200 - 1;
    short bits[] = {Utils::getBit(value, 1), Utils::getBit(value, 2), Utils::getBit(value, 4), Utils::getBit(value, 8)};

    for (int i = 0; i < 4; i++) {
        _request.set_value(
            _microstep_offsets[i], bits[i] == pow(i, 2) ? gpiod::line::value::INACTIVE : gpiod::line::value::ACTIVE
        );
    }
}

void StepperController::setEnabled(bool value) {
    _enabled = value;
    _request.set_value(_dir_offset, value ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
}

bool StepperController::isEnabled() {
    return _enabled;
}