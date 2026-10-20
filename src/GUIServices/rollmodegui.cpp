#include "rollmodegui.h"

namespace constants
{
    // center of screen x and y coords, also where the center big dice lives
    constexpr uint16_t SCREEN_CENTER_X = 160;
    constexpr uint16_t SCREEN_CENTER_Y = 120;
    constexpr uint16_t SCREEN_WIDTH = 320;
    constexpr uint16_t SCREEN_HEIGHT = 240;

    // center of where the left die goes
    constexpr uint16_t LEFT_DIE_X_CENTER = 40;
    constexpr uint16_t LEFT_DIE_Y_CENTER = 120;
    // center of where the right die goes
    constexpr uint16_t RIGHT_DIE_X_CENTER = 280;
    constexpr uint16_t RIGHT_DIE_Y_CENTER = 120;
    // first die in the dice line on bottom of screen, center point
    constexpr uint16_t FIRST_LIL_DIE_X_CENTER = 23;
    constexpr uint16_t FIRST_LIL_DIE_Y_CENTER = 212;

    // used for placement of sprites so we don't have to calculate it every time
    constexpr uint16_t LEFT_DIE_X = LEFT_DIE_X_CENTER - (Images::medDieSpriteWidth/2);
    constexpr uint16_t LEFT_DIE_Y = LEFT_DIE_Y_CENTER - (Images::medDieSpriteHeight/2);

    constexpr uint16_t CENTER_DIE_X = SCREEN_CENTER_X - (Images::bigDieSpriteWidth/2);
    constexpr uint16_t CENTER_DIE_Y = SCREEN_CENTER_Y - (Images::bigDieSpriteHeight/2);

    constexpr uint16_t RIGHT_DIE_X = RIGHT_DIE_X_CENTER - (Images::medDieSpriteWidth/2);
    constexpr uint16_t RIGHT_DIE_Y = RIGHT_DIE_Y_CENTER - (Images::medDieSpriteHeight/2);

    constexpr uint16_t FIRST_LIL_DIE_X = FIRST_LIL_DIE_X_CENTER - (Images::lilDieSpriteWidth/2);
    constexpr uint16_t FIRST_LIL_DIE_Y = FIRST_LIL_DIE_Y_CENTER - (Images::lilDieSpriteHeight/2);

    // this is the box where we can put dice into in the result screen
    // constexpr uint16_t RESULT_PLACEMENT_X = 5;
    // constexpr uint16_t RESULT_PLACEMENT_Y = 1;
    // constexpr uint16_t RESULT_PLACEMENT_WIDTH = 311;
    // constexpr uint16_t RESULT_PLACEMENT_HEIGHT = 210;
        constexpr uint16_t RESULT_PLACEMENT_X = 20;
    constexpr uint16_t RESULT_PLACEMENT_Y = 0;
    constexpr uint16_t RESULT_PLACEMENT_WIDTH = 181;
    constexpr uint16_t RESULT_PLACEMENT_HEIGHT = 240;

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
    _lilDice.setTextDatum(CC_DATUM);

    _medDice.setTextSize(2);
    _medDice.setTextColor(TFT_WHITE);
    _medDice.setTextFont(1);
    _medDice.setTextDatum(CC_DATUM);

    _dieAmounts = {0, 0, 0, 0, 0, 0};
    _dieTypes = {20, 12, 10, 8, 6, 4};
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
        _navigation = Functions::wrapAround<int>(_navigation + navDelta, 0, _dieAmounts.size() - 1);
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
        drawResultBackground();
        populateResultScreen(
            static_cast<int>(constants::RESULT_PLACEMENT_X), 
            static_cast<int>(constants::RESULT_PLACEMENT_Y), 
            static_cast<int>(constants::RESULT_PLACEMENT_WIDTH), 
            static_cast<int>(constants::RESULT_PLACEMENT_HEIGHT), 
            _lilDice,
            Images::lilDiceWidth, Images::lilDiceHeight, 
            Images::lilDice, 
            15, 0
        );
        // populateResultScreen(_medDice, Images::medDiceWidth, Images::medDiceHeight, Images::medDice);
        // populateResultScreen(_medDice, Images::medDiceWidth, Images::medDiceHeight, Images::medDice, 10, 10);

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
    int navToDec = Functions::wrapAround<int>(_navigation - 1, 0, _dieAmounts.size() - 1);
    int navToInc = Functions::wrapAround<int>(_navigation + 1, 0, _dieAmounts.size() - 1);

    // push the new three dice. for these we use _navigation which tells us where to go
    //first draw left dice
    pushDiceSprite(_medDice, constants::LEFT_DIE_X, constants::LEFT_DIE_Y, *Images::medDiceWidth[navToDec], *Images::medDiceHeight[navToDec], Images::medDice[navToDec]);
    pushDiceSprite(_bigDice, constants::CENTER_DIE_X, constants::CENTER_DIE_Y, *Images::bigDiceWidth[_navigation], *Images::bigDiceHeight[_navigation], Images::bigDice[_navigation]);
    pushDiceSprite(_medDice, constants::RIGHT_DIE_X, constants::RIGHT_DIE_Y, *Images::medDiceWidth[navToInc], *Images::medDiceHeight[navToInc], Images::medDice[navToInc]);
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
    for(int i = 0; i < _dieAmounts.size(); i++)
    {
        if(_dieAmounts[i] > 0)
        {
            // set up char array to display. we must convert int to char array
            char buffer[6];
            snprintf(buffer, 6, "x%u", _dieAmounts[i]);
            pushDiceSprite(
                _lilDice,
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE,
                constants::FIRST_LIL_DIE_Y,
                *Images::lilDiceWidth[i],
                *Images::lilDiceHeight[i],
                Images::lilDice[i],
                buffer
            );
            placement++;
        }
    }
}
void RollModeGUI::pushBottomDice(std::array<uint8_t, 6> prevDieAmounts)
{
    for(int i = 0; i < _dieAmounts.size(); i++)
    {
        // if any dice need to be completely removed or added, just call pushBottomDice() to reset the whole thing
        if((prevDieAmounts[i] == 0) != (_dieAmounts[i] == 0))
        {
            pushBottomDice();
            return;
        }
    }
    uint8_t placement = 0;
    for(int i = 0; i < _dieAmounts.size(); i++)
    {
        if(prevDieAmounts[i] != _dieAmounts[i])
        {
            // set up char array to display. we must convert int to char array
            char buffer[6];
            snprintf(buffer, 6, "x%u", _dieAmounts[i]);
            pushDiceSprite(
                _lilDice,
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE,
                constants::FIRST_LIL_DIE_Y,
                *Images::lilDiceWidth[i],
                *Images::lilDiceHeight[i],
                Images::lilDice[i],
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
        constants::FIRST_LIL_DIE_X,
        constants::FIRST_LIL_DIE_Y,
        constants::SCREEN_WIDTH, // screen width
        Images::lilDieSpriteHeight,
        TFT_BLACK
    );
}

void RollModeGUI::populateResultScreen(int resultBoxX, int resultBoxY, int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, const uint16_t* const* imageWidths, const uint16_t* const* imageHeights, const uint16_t* const* images)
{
    // num dice per row/col is a simple division. since divion rounds toward 0, we will always have leftover or a perfect fit
    int numDicePerRow = resultBoxWidth/sprite.width();
    int numDicePerCol = resultBoxHeight/sprite.height(); 
    // we find the remainder of the previous calculation which gives us the amount of pixels we have left to use after space 
    // is taken by the amount of sprites we use. we divide that by the amount of die we are using (-1 since the last die in 
    //the sequence does not need the padding) to get the space between sprites
    int colGapSize = (resultBoxWidth%sprite.width())/(numDicePerRow - 1);
    int rowGapSize = (resultBoxHeight%sprite.height())/(numDicePerCol - 1);
    // finnally this equation gives us the amount of pixels we have left after all that. 
    // split it in half to find left padding and top padding. because of the way it populates,
    // we do not need to save the right or bottom padding because. 
    int paddingLeft = ((resultBoxWidth%sprite.width())%(numDicePerRow - 1))/2;
    int paddingTop = ((resultBoxHeight%sprite.height())%(numDicePerCol - 1))/2;

    int currentXPlacement = resultBoxX + paddingLeft;
    int currentYPlacement = resultBoxY + paddingTop;
    // set up char array to display. we must convert int to char array
    char buffer[6];
    
    // loop through each die we have (d20, d12, d10 etc)
    for(int i = 0; i < _dieAmounts.size(); i++)
    {
        // roll i dice j times
        for(int j = 0; j < _dieAmounts[i]; j++)
        {
            // if we go over the bounds we have set, reset x and inc y, (if the next die will go over the box we've set)
            if (currentXPlacement + sprite.width() > resultBoxWidth + resultBoxX)
            {
                currentXPlacement = resultBoxX + paddingLeft;
                currentYPlacement += sprite.height() + rowGapSize;
            }
            // push the dice using the coordinates weve set
            snprintf(buffer, 6, "%u", random(1, _dieTypes[i] + 1));
            pushDiceSprite(sprite, currentXPlacement, currentYPlacement, *imageWidths[i], *imageHeights[i], images[i], buffer);
            
            // inc x, make sure to add the padding
            currentXPlacement += sprite.width() + colGapSize;
            // delay between each push for visual effect
            delay(10);
        }
    }
}

void RollModeGUI::populateResultScreen(int resultBoxX, int resultBoxY, int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, const uint16_t* const* imageWidths, const uint16_t* const* imageHeights, const uint16_t* const* images, int colGapSize, int rowGapSize)
{
    // since we are basing amount of dice we can place based in a row or col based on the padding, not the other way around, the calculations are a little different
    // divide total area by width of sprite + gap size. The final die in row/col does NOT place the gap padding to its right like the others
    // meaning there is one less "gap" then there are sprites being pushed per row/col. to ammend this, adding the gap size to width/height 
    // of the placement area effectivly cancels out one gap, giving us an accurate calculation of have many dice can properly fit with the given gap.
    // sorry future me or other people that see this that find that explanation confusing, I'm not great at explaining the math in my head.
    // but I'm sure you're smart soooooooooo you probably didn't really need the explanation anyway LOL. 
    int numDicePerRow = (resultBoxWidth + colGapSize)/(sprite.width() + colGapSize);
    int numDicePerCol = (resultBoxHeight + rowGapSize)/(sprite.height() + rowGapSize); 
    // this is similar to the standard version to find the padding, we just need to find how many pixels are remaining
    // after the previous equation an cut it in half. 
    int paddingLeft = ((resultBoxWidth + colGapSize)%(sprite.width() + colGapSize))/2;
    int paddingTop = ((resultBoxHeight + rowGapSize)%(sprite.height() + rowGapSize))/2;

    _tft.drawRect(resultBoxX, resultBoxY, resultBoxWidth, resultBoxHeight, TFT_WHITE);

    int currentXPlacement = resultBoxX + paddingLeft;
    int currentYPlacement = resultBoxY + paddingTop;
    // set up char array to display. we must convert int to char array
    char buffer[6];
    
    // loop through each die we have (d20, d12, d10 etc)
    for(int i = 0; i < _dieAmounts.size(); i++)
    {      
        // roll i dice j times
        for(int j = 0; j < _dieAmounts[i]; j++)
        {
            // if we go over the bounds we have set, reset x and inc y, (if the next die will go over the box we've set)
            if (currentXPlacement + sprite.width() > resultBoxWidth + resultBoxX)
            {
                currentXPlacement = resultBoxX + paddingLeft;
                currentYPlacement += sprite.height() + rowGapSize;
            }
            snprintf(buffer, 6, "%u", random(1, _dieTypes[i] + 1));
            pushDiceSprite(sprite, currentXPlacement, currentYPlacement, *imageWidths[i], *imageHeights[i], images[i], buffer);
            
            currentXPlacement += sprite.width() + colGapSize;
            delay(10);
        }
    }
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
    //push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place 
        x,
        y 
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
    //push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place 
        x,
        y 
    );
}