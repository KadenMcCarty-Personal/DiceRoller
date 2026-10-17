#pragma once
#include <stdint.h>

// defines what pins go to what hardware
// pins for tft must be changed in platform.ini
namespace Pins
{
    constexpr uint8_t kButtonLeft = 7;
    constexpr uint8_t kButtonRight = 6;
    constexpr uint8_t kButtonAdd = 7;
    constexpr uint8_t kButtonSubtract = 7;
    constexpr uint8_t kButtonRoll = 7;
    constexpr uint8_t kButtonMode = 2;
}