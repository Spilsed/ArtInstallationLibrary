#include "LimitSwitch.h"

LimitSwitch::LimitSwitch(
    unsigned int pin, 
    const std::string &chip_path)
    : _chip(chip_path),
      _pin(pin),
      
      _request(_chip.prepare_request()
                .set_request_config(gpiod::request_config().set_consumer("limit_switch"))
                .add_line_settings(
                    pin,
                    gpiod::line_settings()
                        .set_direction(gpiod::line::direction::OUTPUT)
                        .set_output_value(gpiod::line::value::INACTIVE)
                )
                .do_request()) 
{}

bool LimitSwitch::get() {
    return _request.get_value(_pin) == gpiod::line::value::INACTIVE;
}