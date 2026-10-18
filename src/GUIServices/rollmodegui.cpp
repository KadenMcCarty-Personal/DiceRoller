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
    // first die in the dice line on bottom of screen, center point
    constexpr uint16_t FIRST_LIL_DIE_X = 23;
    constexpr uint16_t FIRST_LIL_DIE_Y = 212;
    // we can only fit so many tiny die. maybe implement pages on the roll screen
    constexpr uint16_t MAX_ON_SCREEN_DIE = 117;
    // The space in pixels between all the lil dice on the bottom of screen. to be clear, this is the distance between each die's center point, not edge. 
    constexpr uint16_t PIXELS_BETWEEN_LIL_BOTTOM_DICE = 55;
}

RollModeGUI::RollModeGUI(TFT_eSPI& tft) : _tft(tft), _bigDice(&tft), _medDice(&tft), _lilDice(&tft){
    _bigDice.createSprite(Images::bigDieSpriteWidth, Images::bigDieSpriteHeight);
    _medDice.createSprite(Images::medDieSpriteWidth, Images::medDieSpriteHeight);

    // init the lil dice sprite, the text there will be different
    _lilDice.createSprite(Images::lilDieSpriteWidth, Images::lilDieSpriteHeight);
    _lilDice.setTextSize(1);
    _lilDice.setTextColor(TFT_WHITE);
    _lilDice.setTextFont(2);
    // center text instead of left allign
    _lilDice.setTextDatum(CC_DATUM);

    _dieAmounts = {0, 0, 0, 0, 0, 0};
    _navigation = 0;
}


void RollModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
{
    // -1 if left, +1 if right 0 if neither or both, for navigation
    // First we are updating navigation 
    int navDelta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
    if (navDelta != 0)
    {
        // loops back to 0/nav max using modulo ("it doesn't matter" -Yuji)
        // ok changed functions.h to a namespace PLEASE WORK IM SO TIRED
        _navigation = Functions::wrapAround<int>(_navigation + navDelta, 0, constants::NAVIGATION_MAX_DEC);
        // after we change which die is selected, update the screen
        pushSelectionDice();
    }

    // change the amount of dice that will be rolled if add or subtract is pressed based on which is selected.
    // add one for + and subtract one for -
    // make sure we dont go into the negatives with constrain
    int addDelta = (subtractPressed ? -1 : 0) + (addPressed ? 1 : 0);
    if (addDelta != 0)
    {
        std::array<uint8_t, 6> prevDieAmounts = _dieAmounts;
        _dieAmounts[_navigation] = constrain(_dieAmounts[_navigation] + addDelta, 0, 255);
        _totalSelected += addDelta;
        pushBottomDice(prevDieAmounts);
    }

    // lastly, check if the roll button has been pressed
    if (rollPressed)
    {

    }
}
void RollModeGUI::enter()
{
    drawBackground();
    pushSelectionDice();
    pushBottomDice();
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
// TOOD: SET UP MAX AND REMOVE x WHEN REACHING TRIPLE DIGITS
void RollModeGUI::pushBottomDice()
{
    // start by resetting the whole bottom
    eraseBottomDice();
    // this is to keep track of where to put the next dit in the line.
    // the reason we do not use i is because there would be large gaps depending on which
    // dice are used. example: a d20 would get put in the first slot but a d4 which is index 5 would
    // get put all the way at the end because of the formula i * PIXELS_BETWEEN_LIL_BOTTOM_DICE
    uint8_t placement = 0;
    for(int i = 0; i < constants::NAVIGATION_MAX; i++)
    {
        if(_dieAmounts[i] > 0)
        {
            // set up char array to display. we must convert int to char array
            char buffer[6];
            snprintf(buffer, 6, "x%u", _dieAmounts[i]);
            pushLilDie(
                i, 
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE, // remember x and y are based off the center of the sprite
                constants::FIRST_LIL_DIE_Y, // the y is constant throughout
                buffer
            );
            placement++;
        }
    }
}
void RollModeGUI::pushBottomDice(std::array<uint8_t, 6> prevDieAmounts)
{
    for(int i = 0; i < constants::NAVIGATION_MAX; i++)
    {
        // if any dice need to be completely removed or added, just call pushBottomDice() to reset the whole thing
        if((prevDieAmounts[i] == 0) != (_dieAmounts[i] == 0))
        {
            pushBottomDice();
            return;
        }
    }
    uint8_t placement = 0;
    for(int i = 0; i < constants::NAVIGATION_MAX; i++)
    {
        if(prevDieAmounts[i] != _dieAmounts[i])
        {
            // set up char array to display. we must convert int to char array
            char buffer[6];
            snprintf(buffer, 6, "x%u", _dieAmounts[i]);
            pushLilDie(
                i, 
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE, // remember x and y are based off the center of the sprite
                constants::FIRST_LIL_DIE_Y, // the y is constant throughout
                buffer
            );
        }
        // make sure we are going to replace a die in the right spot
        placement += (_dieAmounts[i] > 0 ? 1 : 0);
    }
}
void RollModeGUI::eraseBottomDice()
{
    _tft.fillRect(
        constants::FIRST_LIL_DIE_X - (_lilDice.width()/2),
        constants::FIRST_LIL_DIE_Y - (_lilDice.height()/2),
        320, // screen width
        Images::lilDieSpriteHeight,
        TFT_BLACK
    );
}

void RollModeGUI::populateResultScreen()
{

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
void RollModeGUI::drawResultBackground()
{
    //SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
    _tft.fillScreen(0x0);
    _tft.drawBitmap(11, 15, Images::image_paint_4_bits, 280, 204, 0xFFFF);
    _tft.drawBitmap(22, 17, Images::image_paint_7_bits, 265, 154, 0xFFFF);
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
        (Images::medDieSpriteWidth - *Images::medDiceWidth[dieIndex])/2, 
        //calculate local y
        (Images::medDieSpriteHeight - *Images::medDiceHeight[dieIndex])/2,
        // image width and height
        *Images::medDiceWidth[dieIndex],
        *Images::medDiceHeight[dieIndex],
        Images::medDice[dieIndex]
    );
    //push sprite to screen
    _medDice.pushSprite(
        // LEFT_DIE_X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (Images::medDieSpriteWidth/2),
        y - (Images::medDieSpriteHeight/2)
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
        (Images::bigDieSpriteWidth - *Images::bigDiceWidth[dieIndex])/2, 
        //calculate local y
        (Images::bigDieSpriteHeight - *Images::bigDiceHeight[dieIndex])/2,
        // image width and height
        *Images::bigDiceWidth[dieIndex],
        *Images::bigDiceHeight[dieIndex],
        Images::bigDice[dieIndex]
    );
    //push sprite to screen
    _bigDice.pushSprite(
        // LEFT_DIE_X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (Images::bigDieSpriteWidth/2),
        y - (Images::bigDieSpriteHeight/2)
    );
}
void RollModeGUI::pushLilDie(uint8_t dieIndex, int32_t x, int32_t y)
{
    // save the dice image's width and height
    int imageWidth = *Images::lilDiceWidth[dieIndex];
    int imageHeight = *Images::lilDiceHeight[dieIndex];
    // uncomment to make sure we wrap around to either side of _navigation. commented because we already do this before its called
    // dieIndex = Functions::wrapAround<int>(dieIndex, 0, constants::NAVIGATION_MAX_DEC);
    //reset sprite
    _lilDice.fillSprite(TFT_BLACK);
    // center the image on the sprite
    _lilDice.pushImage(
        // caluculate where the image must be placed on the sprite
        (_lilDice.height() - imageWidth)/2, 
        (_lilDice.width() - imageHeight)/2,
        // image width and height
        imageWidth,
        imageHeight,
        Images::lilDice[dieIndex]
    );
    //push sprite to screen
    _lilDice.pushSprite(
        // LEFT_DIE_X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (_lilDice.width()/2),
        y - (_lilDice.height()/2)
    );
}
void RollModeGUI::pushLilDie(uint8_t dieIndex, int32_t x, int32_t y, char printValue[])
{
    // save the dice image's width and height
    int imageWidth = *Images::lilDiceWidth[dieIndex];
    int imageHeight = *Images::lilDiceHeight[dieIndex];
    // uncomment to make sure we wrap around to either side of _navigation. commented because we already do this before its called
    // dieIndex = Functions::wrapAround<int>(dieIndex, 0, constants::NAVIGATION_MAX_DEC);
    //reset sprite
    _lilDice.fillSprite(TFT_BLACK);
    // center the image on the sprite
    _lilDice.pushImage(
        // caluculate where the image must be placed on the sprite
        (_lilDice.width() - imageWidth)/2, 
        (_lilDice.height() - imageHeight)/2,
        // image width and height
        imageWidth,
        imageHeight,
        Images::lilDice[dieIndex]
    );
    _lilDice.drawString(printValue, _lilDice.width()/2 + 1, _lilDice.height()/2);
    //push sprite to screen
    _lilDice.pushSprite(
        // LEFT_DIE_X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (_lilDice.width()/2),
        y - (_lilDice.height()/2)
    );
}

void RollModeGUI::pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, int32_t imageWidth, int32_t imageHeight, const uint16_t *image)
{
    //reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - imageWidth)/2, 
        (sprite.height() - imageHeight)/2,
        imageWidth,
        imageHeight,
        image
    );
    //push sprite to screen
    sprite.pushSprite(
        // X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (sprite.width()/2),
        y - (sprite.height()/2)
    );
}
void RollModeGUI::pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, int32_t imageWidth, int32_t imageHeight, const uint16_t *image, char printValue[])
{
    //reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - imageWidth)/2, 
        (sprite.height() - imageHeight)/2,
        imageWidth,
        imageHeight,
        image
    );
    sprite.drawString(printValue, sprite.width()/2 + 1, sprite.height()/2);
    //push sprite to screen
    sprite.pushSprite(
        // X and Y are the center points for where the dice need to be but we push the sprite
        // from its top left corner so we need to move the center x and y there using width and height
        x - (sprite.width()/2),
        y - (sprite.height()/2)
    );
}