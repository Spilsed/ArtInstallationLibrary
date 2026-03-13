#include "Utils.h"

namespace Utils {
    double lerp(double current, double target, double t) {
        return current * (1.0 - t) + (target * t);
    }

    short getBit(short value, short bit) {
        return (value & ( bit << bit )) >> bit
    }
}