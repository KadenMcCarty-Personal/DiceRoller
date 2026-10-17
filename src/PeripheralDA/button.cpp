
#include "button.h"

//constructor
Button::Button(uint8_t pin) { _pin = pin; }

// begin listening and set up pin state
void Button::begin() 
{
  pinMode(_pin, INPUT_PULLUP);
  _prev = _curr = _currRaw = digitalRead(_pin);
}


// updates button state
void Button::update()
{
  _prev = _curr; 
  bool reading = digitalRead(_pin);
  // if the state has changed, update raw state and note run time to compare later
  if(reading != _currRaw)
  {
    _currRaw = reading; 
    _lastChanged = millis();
  }
  // the button needs to be held this long to actually fire
  if(millis() - _lastChanged >= 10)
  {
    _curr = _currRaw;
  }
}
// check if button was pressed, fires only once
bool Button::pressed()
{
  return (_prev != _curr && _curr == LOW);
}