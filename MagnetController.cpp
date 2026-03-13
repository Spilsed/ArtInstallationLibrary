#include "MagnetController.h"

MagnetController::MagnetController(unsigned int pin, 
                                    const std::string &chip_path)
    : _chip(chip_path),
      _pin(pin),
      
      _request(_chip.prepare_request()
                .set_request_config(gpiod::request_config().set_consumer("magnet_ctrl"))
                .add_line_settings(
                    pin,
                    gpiod::line_settings()
                        .set_direction(gpiod::line::direction::OUTPUT)
                        .set_output_value(gpiod::line::value::INACTIVE)
                )
                .do_request()) 
{}

void MagnetController::set(bool value) {
    _request.set_value(_pin, value ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    _active = value;
}