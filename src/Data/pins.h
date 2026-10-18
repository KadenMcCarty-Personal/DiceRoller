#pragma once
#include <stdint.h>

// defines what pins go to what hardware
// pins for tft must be changed in platform.ini
namespace Pins
{
    constexpr uint8_t kButtonLeft = 7;
    constexpr uint8_t kButtonRight = 6;
    constexpr uint8_t kButtonSubtract = 5;
    constexpr uint8_t kButtonAdd = 4;
    constexpr uint8_t kButtonRoll = 3;
    constexpr uint8_t kButtonMode = 2;
}