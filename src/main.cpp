#include <Arduino.h>
#include <TFT_eSPI.h>

#include "PeripheralDA/button.h"
#include "PeripheralDA/lcdinterface.h"
#include "GUIServices/rollmodegui.h"
#include "GUIServices/savemodegui.h"
#include "GUIServices/attackmodegui.h"
#include "Data/pins.h"
#include "Data/images.h"

//create button objects
Button buttonLeft(Pins::kButtonLeft);
Button buttonRight(Pins::kButtonRight);
Button buttonAdd(Pins::kButtonAdd);
Button buttonSubtract(Pins::kButtonSubtract);
Button buttonRoll(Pins::kButtonRoll);
Button buttonMode(Pins::kButtonMode);

// create the tft spi object
TFT_eSPI tft = TFT_eSPI(); 

// init gui service objects
LcdInterface lcdInterface(tft);
RollModeGUI rollMode(lcdInterface);
SaveModeGUI saveMode(lcdInterface);
AttackModeGUI attackMode(lcdInterface);

// total number of elements in the enum MUST UPDATE WHEN ADDING MORE. doing this so it does not
// have to be calculated every time mode is called or calculated at all
int8_t modeAmount = 3;
// enum for modes
enum class Mode {
  ROLL, 
  SAVE,
  ATTACK
};
// set mode
Mode mode = Mode::ROLL;
// we use modulo (JJK REFERENCE) of this number to cycle throught the modes
int16_t modeCycleInt = 0; 

//init functions
void cycleMode(bool isReady);
void update();

void setup() 
{
  // TESTING
  Serial.begin(115200);
  // TESTING

  // set pin modes
  pinMode(Pins::kButtonLeft, INPUT_PULLUP);
  pinMode(Pins::kButtonRight, INPUT_PULLUP);
  pinMode(Pins::kButtonMode, INPUT_PULLUP);

  // init lcd interface
  lcdInterface.begin();

  // init buttons
  buttonLeft.begin();
  buttonRight.begin();
  buttonAdd.begin();
  buttonSubtract.begin();
  buttonRoll.begin();
  buttonMode.begin();

  // we start in roll mode so we use its enter function here
  rollMode.enter();
}

void loop() {
  // update buttons
  buttonLeft.update();
  buttonRight.update();
  buttonAdd.update();
  buttonSubtract.update();
  buttonRoll.update();
  buttonMode.update();

  // wip
  cycleMode(buttonMode.pressed());
  // wip
  update();
  
}
// wip
void cycleMode(bool isReady)
{
  //check if button has been pressed
  if (isReady)
  {
    // add to the cycle
    modeCycleInt++; 
    mode = static_cast<Mode>(modeAmount%modeCycleInt);
    // depending on the mode, call the correct enter fun
    switch (mode){
      case Mode::ROLL:
        rollMode.enter();
        break;

      case Mode::SAVE:
        saveMode.enter();
        break;

      case Mode::ATTACK:
        attackMode.enter();
        break;
      default:
        break;
    }
  }
}
// wip
void update()
{
  switch (mode){
    case Mode::ROLL:
      // update function for roll mode
      rollMode.update(buttonLeft.pressed(), buttonRight.pressed(), buttonAdd.pressed(), buttonSubtract.pressed(), buttonRoll.pressed());
      break;

    case Mode::SAVE:

      break;

    case Mode::ATTACK:

      break;
    default:
      break;

  }
}
