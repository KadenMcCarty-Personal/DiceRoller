#include "rollmodegui.h"

namespace constants
{
    // set this to one less than the actual max since it is mostly used in arrays where
    // the first element is 0
    constexpr uint8_t NAVIGATION_MAX_DEC = 5;
    constexpr uint8_t NAVIGATION_MAX = 6;
    // center of screen x and y coords, also where the center big dice lives
    constexpr uint16_t SCREEN_CENTER_X = 160;
    constexpr uint16_t SCREEN_CENTER_Y = 120;
    // center of where the left die goes
    constexpr uint16_t LEFT_DIE_X = 40;
    constexpr uint16_t LEFT_DIE_Y = 120;
    // center of where the right die goes
    constexpr uint16_t RIGHT_DIE_X = 280;
    constexpr uint16_t RIGHT_DIE_Y = 120;
    // we can only fit so many tiny die. maybe implement pages on the roll screen
    constexpr uint16_t MAX_ON_SCREEN_DIE = 117;
}

RollModeGUI::RollModeGUI(LcdInterface& lcdInterface) : _lcdInterface(lcdInterface) {
    _dieAmounts = {0, 0, 0, 0, 0, 0};
    _navigation = 0;
}

void RollModeGUI::navigate(uint8_t navFrom)
{
    // helper function to erase the dice of the state we are coming from
    eraseSelectionDice(navFrom);
    drawSelectionDice();
}

void RollModeGUI::eraseSelectionDice(uint8_t navFrom)
{
    // find the navigation of the dice to the left and right of the one we are selecting.
    // we do this here so we only have to do wrap around func once once for each
    int navFromDec = Functions::wrapAround<int>(navFrom - 1, 0, constants::NAVIGATION_MAX_DEC);
    int navFromInc = Functions::wrapAround<int>(navFrom + 1, 0, constants::NAVIGATION_MAX_DEC);


    // first erase the three dice that are currently on screen
    // draw black over the left die perfectly
    _lcdInterface.drawBlackRect(
        constants::LEFT_DIE_X - (*Images::medDiceWidth[navFromDec] / 2), 
        constants::LEFT_DIE_Y - (*Images::medDiceHeight[navFromDec] / 2),
        *Images::medDiceWidth[navFromDec],
        *Images::medDiceHeight[navFromDec]
    );
    // now draw a perfect black rect around whatever die is centered on the screen
    _lcdInterface.drawBlackRect(
        constants::SCREEN_CENTER_X - (*Images::bigDiceWidth[navFrom] / 2), 
        constants::SCREEN_CENTER_Y - (*Images::bigDiceHeight[navFrom] / 2),
        *Images::bigDiceWidth[navFrom],
        *Images::bigDiceHeight[navFrom]
    );
    // draw black over the right die perfectly
    _lcdInterface.drawBlackRect(
        constants::RIGHT_DIE_X - (*Images::medDiceWidth[navFromInc] / 2), 
        constants::RIGHT_DIE_Y - (*Images::medDiceHeight[navFromInc] / 2),
        *Images::medDiceWidth[navFromInc],
        *Images::medDiceHeight[navFromInc]
    );
}
void RollModeGUI::drawSelectionDice()
{
    // find the navigation of the dice to the left and right of the one we are selecting.
    // we do this here so we only have to do wrap around func once for each
    int navToDec = Functions::wrapAround<int>(_navigation - 1, 0, constants::NAVIGATION_MAX_DEC);
    int navToInc = Functions::wrapAround<int>(_navigation + 1, 0, constants::NAVIGATION_MAX_DEC);

    // push the new three dice. for these we use _navigation which tells us where to go
    //first draw left dice
    _lcdInterface.pushImage(
        constants::LEFT_DIE_X - (*Images::medDiceWidth[navToDec] / 2), 
        constants::LEFT_DIE_Y - (*Images::medDiceHeight[navToDec] / 2),
        *Images::medDiceWidth[navToDec],
        *Images::medDiceHeight[navToDec],
        Images::medDice[navToDec]
    );
    // then center big dice
    _lcdInterface.pushImage(
        constants::SCREEN_CENTER_X - (*Images::bigDiceWidth[_navigation] / 2), 
        constants::SCREEN_CENTER_Y - (*Images::bigDiceHeight[_navigation] / 2),
        *Images::bigDiceWidth[_navigation],
        *Images::bigDiceHeight[_navigation],
        Images::bigDice[_navigation]
    );
    // then right dice
    _lcdInterface.pushImage(
        constants::RIGHT_DIE_X - (*Images::medDiceWidth[navToInc] / 2), 
        constants::RIGHT_DIE_Y - (*Images::medDiceHeight[navToInc] / 2),
        *Images::medDiceWidth[navToInc],
        *Images::medDiceHeight[navToInc],
        Images::medDice[navToInc]
    );
}
void RollModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
{
    // -1 if left, +1 if right 0 if neither or both, for navigation
    // writen in inline ifs for readability, same with later
    int delta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
    if (delta != 0)
    {
        // we need to pass where we are navigating from into the navigate function
        int navFrom = _navigation;
        // loops back to 0/nav max using modulo ("it doesn't matter" -Yuji)
        // ok changed functions.h to a namespace PLEASE WORK IM SO TIRED
        _navigation = Functions::wrapAround<int>(_navigation + delta, 0, constants::NAVIGATION_MAX_DEC);
        // while we could skip the navFrom int and just put the eraser function before we change _navigation
        // and put draw function after, I found it to be cleaner to have it put in one place that can call both functions
        navigate(navFrom);
    }
    // change the amount of dice that will be rolled based on which is selected.
    // add one for + and subtract one for -
    // make sure we dont go into the negatives with constrain
    _dieAmounts[_navigation] = constrain(_dieAmounts[_navigation] + ((subtractPressed ? -1 : 0) + (addPressed ? 1 : 0)), 0, 255);
}
void RollModeGUI::enter()
{
    _lcdInterface.drawRollModeBackground();
    drawSelectionDice();
}
