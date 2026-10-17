#pragma once
#include <Arduino.h>

class Button 
{
  public:
    Button(uint8_t pin);
    void begin();
    void update();
    bool pressed();
  private:
    uint8_t _pin;
    bool _prev;
    bool _curr;
    bool _currRaw;
    unsigned long _lastChanged;
};