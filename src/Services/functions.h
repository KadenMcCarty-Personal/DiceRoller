#pragma once
#include <Arduino.h>


namespace Functions
{
    /// @brief if number is below min, wraps around back to max and returns it
    /// @param num 
    /// @param min inclusive
    /// @param max inclusive
    /// @return 
    template <typename T>
    constexpr T wrapAround(T num, T min, T max)
    {
        return num < min ? max : (num > max ? min : num);
    }
    /// @brief works only for positives (i think)
    /// @param a 
    /// @param b 
    /// @return 
    constexpr int divideRoundUp(int a, int b)
    {
        return (a + b - 1) / b;
    }
}