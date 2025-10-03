#include "controller.h" // Include the header with declarations
#include <utility>

// Constructor implementation
Controller::Controller()
    : chip(nullptr), request(nullptr), running(false), pulse_width_us(0), gpio_pin(0) {}

// Destructor implementation
Controller::~Controller() {
    cleanup();
}

// pwmLoop implementation
void Controller::pwmLoop() {
    while (running) {
        int pw = pulse_width_us.load();
        
        if (!request) { 
            std::cerr << "PWM Error: GPIO line request is null." << std::endl;
            break;
        }

        if (pw > 0) {
            try {
                // High phase
                request->set_value(gpio_pin, gpiod::line::value::ACTIVE);
                std::this_thread::sleep_for(std::chrono::microseconds(pw));

                // Low phase
                request->set_value(gpio_pin, gpiod::line::value::INACTIVE);
                int remaining_time_us = PWM_PERIOD_US - pw;
                if (remaining_time_us > 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(remaining_time_us));
                }
            } catch (const std::exception &e) {
                std::cerr << "PWM Error in thread: " << e.what() << std::endl;
                break;
            }
        } else {
            // Keep pin low
            try {
                request->set_value(gpio_pin, gpiod::line::value::INACTIVE);
            } catch (const std::exception &e) {
                std::cerr << "PWM Error on stop: " << e.what() << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(PWM_PERIOD_US));
        }
    }
}

// initialize implementation
bool Controller::initialize(unsigned int pin, const char *chip_path) {
    try {
        gpio_pin = pin;

        // Allocate and open the GPIO chip
        chip = new gpiod::chip(chip_path);
        std::cout << "Opened GPIO chip: " << chip->get_info().name() << std::endl;
        std::cout << "Label: " << chip->get_info().label() << std::endl;

        // Configure line as an output
        gpiod::line_settings settings;
        settings.set_direction(gpiod::line::direction::OUTPUT)
            .set_output_value(gpiod::line::value::INACTIVE);

        // Allocate and request the line
        request = new gpiod::line_request(
            chip->prepare_request()
                .set_consumer("servo_controller")
                .add_line_settings(gpio_pin, settings)
                .do_request()
        );
        
        std::cout << "Configured GPIO " << gpio_pin << " for servo control" << std::endl;

        running = true;
        pwm_thread = std::thread(&Controller::pwmLoop, this);
        
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        cleanup();
        return false;
    }
}

// setSpeed implementation
void Controller::setSpeed(int speed_percent) {
    if (speed_percent < -100) speed_percent = -100;
    if (speed_percent > 100) speed_percent = 100;

    // Maps -100% to 1000us, 0% to 1500us, 100% to 2000us
    int pw = 1500 + (speed_percent * 500 / 100);
    pulse_width_us.store(pw);

    std::cout << "Speed: " << speed_percent << "% (pulse: " << pw << "us)" << std::endl;
}

// stop implementation
void Controller::stop() {
    setSpeed(0);
}

// cleanup implementation
void Controller::cleanup() {
    if (running) {
        // Set neutral pulse and wait briefly
        pulse_width_us.store(1500); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        running = false;
        if (pwm_thread.joinable()) {
            pwm_thread.join();
        }
    }

    // Explicitly delete raw pointers
    try {
        if (request) {
            request->set_value(gpio_pin, gpiod::line::value::INACTIVE);
            delete request; 
            request = nullptr;
        }
        if (chip) {
            delete chip; 
            chip = nullptr;
        }
    } catch (const std::exception &e) {
        std::cerr << "Cleanup error: " << e.what() << std::endl;
    }
}
