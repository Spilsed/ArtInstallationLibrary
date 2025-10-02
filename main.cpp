#include <csignal>
#include <iostream>
#include <thread>
#include <chrono>

#include "controller.h" // Include the corrected header

// Global Controller instance
Controller servo;

void signalHandler(int signum) {
    std::cout << std::endl << "Interrupt signal (" << signum << ") recieved! Shutting down..." << std::endl;

    // Stop and cleanup logic is already handled here
    servo.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    servo.cleanup();

    exit(signum);
}

int main() {
    std::cout << "Rpi5 Controller Application" << std::endl;

    // Set up signal handlers for graceful shutdown (Ctrl+C and termination)
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // *** IMPORTANT: Change 25 to your actual GPIO line offset ***
    const unsigned int GPIO_PIN = 25; 

    if (!servo.initialize(GPIO_PIN, "/dev/gpiochip4")) {
        std::cerr << "FATAL: Failed to initialize Controller. Check GPIO chip path and pin." << std::endl;
        return 1;
    }

    std::cout << "Controller initialized. Running test sequence (5 seconds)..." << std::endl;
    
    try {
        // Simple test sequence to demonstrate control
        servo.setSpeed(75);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        servo.stop();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        servo.setSpeed(-50);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        servo.stop();
    } catch (const std::exception& e) {
        std::cerr << "Runtime error in main: " << e.what() << std::endl;
    }

    // Wait 1 second for the final stop pulse to complete before cleaning up
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Explicit cleanup
    std::cout << "Test sequence finished. Cleaning up resources." << std::endl;
    servo.cleanup();

    return 0;
}
