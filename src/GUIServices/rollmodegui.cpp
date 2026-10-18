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

RollModeGUI::RollModeGUI(TFT_eSPI& tft) : _tft(tft), _bigDice(&tft), _medDice(&tft){
    _bigDice.createSprite(Images::maxBigDieWidth, Images::maxBigDieHeight);
    _medDice.createSprite(Images::maxMedDieWidth, Images::maxMedDieHeight);
    _dieAmounts = {0, 0, 0, 0, 0, 0};
    _navigation = 0;
}
// MIGHT BE REMOVED SOON
void RollModeGUI::navigate()
{
    pushSelectionDice();
}

void RollModeGUI::eraseSelectionDice(uint8_t navFrom)
{
    // find the navigation of the dice to the left and right of the one we are selecting.
    // we do this here so we only have to do wrap around func once once for each
    int navFromDec = Functions::wrapAround<int>(navFrom - 1, 0, constants::NAVIGATION_MAX_DEC);
    int navFromInc = Functions::wrapAround<int>(navFrom + 1, 0, constants::NAVIGATION_MAX_DEC);


    // first erase the three dice that are currently on screen
    // draw black over the left die perfectly
    _tft.fillRect(
        constants::LEFT_DIE_X - (*Images::medDiceWidth[navFromDec] / 2), 
        constants::LEFT_DIE_Y - (*Images::medDiceHeight[navFromDec] / 2),
        *Images::medDiceWidth[navFromDec],
        *Images::medDiceHeight[navFromDec],
        TFT_BLACK
    );
    // now draw a perfect black rect around whatever die is centered on the screen
    _tft.fillRect(
        constants::SCREEN_CENTER_X - (*Images::bigDiceWidth[navFrom] / 2), 
        constants::SCREEN_CENTER_Y - (*Images::bigDiceHeight[navFrom] / 2),
        *Images::bigDiceWidth[navFrom],
        *Images::bigDiceHeight[navFrom],
        TFT_BLACK
    );
    // draw black over the right die perfectly
    _tft.fillRect(
        constants::RIGHT_DIE_X - (*Images::medDiceWidth[navFromInc] / 2), 
        constants::RIGHT_DIE_Y - (*Images::medDiceHeight[navFromInc] / 2),
        *Images::medDiceWidth[navFromInc],
        *Images::medDiceHeight[navFromInc],
        TFT_BLACK
    );
}
void RollModeGUI::pushSelectionDice()
{
    // find the navigation of the dice to the left and right of the one we are selecting.
    // we do this here so we only have to do wrap around func once for each
    int navToDec = Functions::wrapAround<int>(_navigation - 1, 0, constants::NAVIGATION_MAX_DEC);
    int navToInc = Functions::wrapAround<int>(_navigation + 1, 0, constants::NAVIGATION_MAX_DEC);

    // push the new three dice. for these we use _navigation which tells us where to go
    //first draw left dice
    pushMedDie(navToDec, constants::LEFT_DIE_X, constants::LEFT_DIE_Y);
    pushBigDie(_navigation, constants::SCREEN_CENTER_X, constants::SCREEN_CENTER_Y);
    pushMedDie(navToInc, constants::RIGHT_DIE_X, constants::RIGHT_DIE_Y);
}
void RollModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
{
    // -1 if left, +1 if right 0 if neither or both, for navigation
    // writen in inline ifs for readability, same with later
    int delta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
    if (delta != 0)
    {
        // loops back to 0/nav max using modulo ("it doesn't matter" -Yuji)
        // ok changed functions.h to a namespace PLEASE WORK IM SO TIRED
        _navigation = Functions::wrapAround<int>(_navigation + delta, 0, constants::NAVIGATION_MAX_DEC);
        // while we could skip the navFrom int and just put the eraser function before we change _navigation
        // and put draw function after, I found it to be cleaner to have it put in one place that can call both functions
        navigate();
    }
    // change the amount of dice that will be rolled based on which is selected.
    // add one for + and subtract one for -
    // make sure we dont go into the negatives with constrain
    _dieAmounts[_navigation] = constrain(_dieAmounts[_navigation] + ((subtractPressed ? -1 : 0) + (addPressed ? 1 : 0)), 0, 255);
}
void RollModeGUI::enter()
{
    drawBackground();
    pushSelectionDice();
}

// ===================================================================
// helper functions for interacting with tft
// ===================================================================
void RollModeGUI::drawBackground()
{
    //SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
  _tft.fillScreen(0x0);
  // ellipse 1
  _tft.drawEllipse(330, 2, 59, 55, 0xFFFF);
  // string 2
  _tft.setTextColor(0xFFFF);
  _tft.setTextSize(2);
  _tft.drawString("Roll Mode", 7, 6);
  // paint 4
  _tft.drawBitmap(11, 15, Images::image_paint_4_bits, 280, 204, 0xFFFF);
  // line 6
  _tft.drawLine(6, 227, 313, 227, 0xFFFF);
  // paint 7
  _tft.drawBitmap(22, 17, Images::image_paint_7_bits, 265, 154, 0xFFFF);
  // ellipse 1
  _tft.drawEllipse(343, -13, 59, 55, 0xFFFF);
  // ButtonLeftSmall
  _tft.drawBitmap(84, 111, Images::image_ButtonLeftSmall_bits, 6, 10, 0xFFFF);
  // ButtonRightSmall
  _tft.drawBitmap(230, 112, Images::image_ButtonRightSmall_bits, 6, 10, 0xFFFF);
}
void RollModeGUI::pushMedDie(uint8_t dieIndex, int32_t x, int32_t y)
{
    // uncomment to make sure we wrap around to either side of _navigation. commented because we already do this before its called
    // dieIndex = Functions::wrapAround<int>(dieIndex, 0, constants::NAVIGATION_MAX_DEC);
    //reset sprite
    _medDice.fillSprite(TFT_BLACK);
    // center the image on the sprite
    _medDice.pushImage(
        // caluculate local x
        (Images::maxMedDieWidth - *Images::medDiceWidth[dieIndex])/2, 
        //calculate local y
        (Images::maxMedDieHeight - *Images::medDiceHeight[dieIndex])/2,
        // image width and height
        *Images::medDiceWidth[dieIndex],
        *Images::medDiceHeight[dieIndex],
        Images::medDice[dieIndex]
    );
    //push sprite to screen
    _medDice.pushSprite(
        // LEFT_DIE_X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (Images::maxMedDieWidth/2),
        y - (Images::maxMedDieHeight/2)
    );
}
void RollModeGUI::pushBigDie(uint8_t dieIndex, int32_t x, int32_t y)
{
    // uncomment to make sure we wrap around to either side of _navigation. commented because we already do this before its called
    // dieIndex = Functions::wrapAround<int>(dieIndex, 0, constants::NAVIGATION_MAX_DEC);
    //reset sprite
    _bigDice.fillSprite(TFT_BLACK);
    // center the image on the sprite
    _bigDice.pushImage(
        // caluculate local x
        (Images::maxBigDieWidth - *Images::bigDiceWidth[dieIndex])/2, 
        //calculate local y
        (Images::maxBigDieHeight - *Images::bigDiceHeight[dieIndex])/2,
        // image width and height
        *Images::bigDiceWidth[dieIndex],
        *Images::bigDiceHeight[dieIndex],
        Images::bigDice[dieIndex]
    );
    //push sprite to screen
    _bigDice.pushSprite(
        // LEFT_DIE_X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (Images::maxBigDieWidth/2),
        y - (Images::maxBigDieHeight/2)
    );
}
