#include "StepperController.h"
#include <thread>
#include <chrono>

using namespace std::chrono;

StepperController::StepperController(unsigned int step_pin, 
                                     unsigned int dir_pin, 
                                     const std::string &chip_path)
    : _chip(chip_path),
      _step_offset(step_pin),
      _dir_offset(dir_pin),

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
    }
}