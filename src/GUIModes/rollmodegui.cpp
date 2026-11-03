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
    constexpr uint16_t LEFT_DIE_X = LEFT_DIE_X_CENTER - (Images::medDieSpriteWidth / 2);
    constexpr uint16_t LEFT_DIE_Y = LEFT_DIE_Y_CENTER - (Images::medDieSpriteHeight / 2);

    constexpr uint16_t CENTER_DIE_X = SCREEN_CENTER_X - (Images::bigDieSpriteWidth / 2);
    constexpr uint16_t CENTER_DIE_Y = SCREEN_CENTER_Y - (Images::bigDieSpriteHeight / 2);

    constexpr uint16_t RIGHT_DIE_X = RIGHT_DIE_X_CENTER - (Images::medDieSpriteWidth / 2);
    constexpr uint16_t RIGHT_DIE_Y = RIGHT_DIE_Y_CENTER - (Images::medDieSpriteHeight / 2);

    constexpr uint16_t FIRST_LIL_DIE_X = FIRST_LIL_DIE_X_CENTER - (Images::lilDieSpriteWidth / 2);
    constexpr uint16_t FIRST_LIL_DIE_Y = FIRST_LIL_DIE_Y_CENTER - (Images::lilDieSpriteHeight / 2);

    // this is the box where we can put dice into in the result screen
    constexpr uint16_t RESULT_PLACEMENT_X = 0;
    constexpr uint16_t RESULT_PLACEMENT_Y = 0;
    constexpr uint16_t RESULT_PLACEMENT_WIDTH = 320;
    constexpr uint16_t RESULT_PLACEMENT_HEIGHT = 210;

    // we can only fit so many tiny die. maybe implement pages on the roll screen
    constexpr uint16_t MAX_ON_SCREEN_DIE = 117;
    // The space in pixels between all the lil dice on the bottom of screen. to be clear, this is the distance between each die's center point, not edge.
    constexpr uint16_t PIXELS_BETWEEN_LIL_BOTTOM_DICE = 55;
}

RollModeGUI::RollModeGUI(TFT_eSPI& tft, LCDServices &lcdServices) :
_tft(tft),
_lcdServices(lcdServices)
{
    _dieAmounts = {0, 0, 0, 0, 0, 0};
    _navigation = 0;
    _amountRolled = 0;
    _amountSelected = 0;
    _rolls = {};
    _screen = Screen::SELECT;
}



void RollModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
{
    switch (_screen)
    {
    case Screen::SELECT:
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
            if (_dieAmounts[_navigation] != constrain(_dieAmounts[_navigation] + addDelta, 0, 255))
            {
                _amountSelected += addDelta;
                _dieAmounts[_navigation] += addDelta;
                pushBottomDice(prevDieAmounts);
            }
        }

        // lastly, check if the roll button has been pressed
        if (rollPressed)
        {
            _screen = Screen::RESULTS;
            _currentResultPage = 0;

            //  if nothing selected, just add the current die we are on to selection and roll, then remove it from selection once rolled. 
            if (_amountSelected == 0)
            {
                _amountSelected++;
                _dieAmounts[_navigation]++;
                rollSelected();
                _amountSelected--;
                _dieAmounts[_navigation]--;
            }
            else
                rollSelected();
            enter();
        }
        break;
    }
    case Screen::RESULTS:
    {
        // if left or right is pressed while in result page we cycle through pages
        int pageDelta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
        // only change pages if the page actually would change
        if (pageDelta != 0 && _amountOfPages > 1 && _currentResultPage != constrain(_currentResultPage + pageDelta, 0, _amountOfPages - 1))
        {
            _currentResultPage = constrain(_currentResultPage + pageDelta, 0, _amountOfPages - 1);
            // after we change which page is selected, update the screen
            drawResultBackground();
            pushResults();
        }

        int addDelta = (subtractPressed ? -1 : 0) + (addPressed ? 1 : 0);
        if (addDelta != 0)
        {
            _modifier += addDelta;
            // TOOD I WANNA DO THIS NOW BUT DONT
            modifyTotal();
        }

        // lastly, check if the roll button has been pressed we switch back to selection screen
        if (rollPressed)
        {
            _screen = Screen::SELECT;
            enter();
        }
        break;
    }
    }
}
void RollModeGUI::enter()
{
    switch (_screen)
    {
    case Screen::SELECT:
        drawBackground();
        pushSelectionDice();
        pushBottomDice();
        break;
    case Screen::RESULTS:
        drawResultBackground();
        pushResults();
        break;
    }
}

void RollModeGUI::rollSelected()
{
    resetRolls();
    // loop through each die we have (d20, d12, d10 etc)
    for (int i = 0; i < _dieAmounts.size(); i++)
    {
        // roll i dice j times
        for (int j = 0; j < _dieAmounts[i]; j++)
        {
            // input roll
            _rolls[_amountRolled] = random(1, DIE_TYPES[i] + 1);
            _rolledDieTypeIndex[_amountRolled] = i;
            _result += _rolls[_amountRolled];
            _amountRolled++;
        }
    }
}
void RollModeGUI::resetRolls()
{
    // set all rolls to zero. no need to go beyond total
    std::fill_n(_rolls.begin(), _amountRolled - 1, 0);
    _amountRolled = 0;
    _result = 0;
    _modifier = 0;
}

void RollModeGUI::pushSelectionDice()
{
    // find the navigation of the dice to the left and right of the one we are selecting.
    // we do this here so we only have to do wrap around func once for each
    int navToDec = Functions::wrapAround<int>(_navigation - 1, 0, _dieAmounts.size() - 1);
    int navToInc = Functions::wrapAround<int>(_navigation + 1, 0, _dieAmounts.size() - 1);

    // push the new three dice. for these we use _navigation which tells us where to go
    // first draw left dice
    _lcdServices.pushDiceSprite(LCDServices::Sprites::MEDDICE, constants::LEFT_DIE_X, constants::LEFT_DIE_Y, Images::medDice[navToDec]);
    _lcdServices.pushDiceSprite(LCDServices::Sprites::BIGDICE, constants::CENTER_DIE_X, constants::CENTER_DIE_Y, Images::bigDice[_navigation]);
    _lcdServices.pushDiceSprite(LCDServices::Sprites::MEDDICE, constants::RIGHT_DIE_X, constants::RIGHT_DIE_Y, Images::medDice[navToInc]);
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
    for (int i = 0; i < _dieAmounts.size(); i++)
    {
        if (_dieAmounts[i] > 0)
        {
            // set up char array to display. we must convert int to char array
            char buffer[6];
            snprintf(buffer, 6, "x%u", _dieAmounts[i]);
            _lcdServices.pushDiceSprite(
                LCDServices::Sprites::LILDICE,
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE,
                constants::FIRST_LIL_DIE_Y,
                Images::lilDice[i],
                buffer);
            placement++;
        }
    }
}
void RollModeGUI::pushBottomDice(std::array<uint8_t, 6> prevDieAmounts)
{
    for (int i = 0; i < _dieAmounts.size(); i++)
    {
        // if any dice need to be completely removed or added, just call pushBottomDice() to reset the whole thing
        if ((prevDieAmounts[i] == 0) != (_dieAmounts[i] == 0))
        {
            pushBottomDice();
            return;
        }
    }
    uint8_t placement = 0;
    for (int i = 0; i < _dieAmounts.size(); i++)
    {
        if (prevDieAmounts[i] != _dieAmounts[i])
        {
            // set up char array to display. we must convert int to char array
            char buffer[6];
            snprintf(buffer, 6, "x%u", _dieAmounts[i]);
            _lcdServices.pushDiceSprite(
                LCDServices::Sprites::LILDICE,
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE,
                constants::FIRST_LIL_DIE_Y,
                Images::lilDice[i],
                buffer);
        }
        // make sure we are going to replace a die in the right spot
        placement += (_dieAmounts[i] > 0 ? 1 : 0);
    }
}


void RollModeGUI::pushResults()
{
    // TOOD (maybe): make this dynamic, magic numbers are typically no go but I may make an exception, we shall see
    if (_amountRolled <= 3)
    {
        _amountOfPages = 1;
        centeredResults(0, LCDServices::Sprites::BIGDICE, Images::bigDice);
    }
    else if (_amountRolled <= 18)
    {
        _amountOfPages = 1;
        centeredResults(0, LCDServices::Sprites::MEDDICE, Images::medDice);
    }
    else
    {
        _amountOfPages = getPageAmount(
            constants::RESULT_PLACEMENT_WIDTH,
            constants::RESULT_PLACEMENT_HEIGHT, LCDServices::Sprites::LILDICE,
            0,
            0);
        centeredResults(_currentResultPage, LCDServices::Sprites::LILDICE, Images::lilDice);
    }
    // maybe not this in future sooooooo keyword!!!! TESTING
    modifyTotal();
}



// MAKRED FOR DEATH
int RollModeGUI::dynamicGapCalculation(int length, int spriteLength)
{
    // we find the remainder of how many times sprite goes into lengths which gives us the amount of pixels we have left to use after space
    // is taken. we divide that by the amount of sprites we are using (-1 since the last die in
    // the sequence does not need the padding) to get the space between sprites
    return (length % spriteLength) / (length / spriteLength - 1);
}

int RollModeGUI::getPageAmount(int resultBoxWidth, int resultBoxHeight, LCDServices::Sprites spriteEnum, int colGapSize, int rowGapSize)
{
    // formula ripped straight from populateResultBox, dividing the total number of dice
    int temp = ((resultBoxWidth + colGapSize) / (_lcdServices.getSpriteWidth(spriteEnum) + colGapSize)) * ((resultBoxHeight + rowGapSize) / (_lcdServices.getSpriteHeight(spriteEnum) + rowGapSize));
    // ngl I was NOT smart enough to come up with this formula so do not credit me. always rounds up
    return (_amountRolled + temp - 1) / temp;
}


void RollModeGUI::centeredResults(int page, LCDServices::Sprites spriteEnum, const Images::Image *images)
{
    // make the box that the results will go into
    GFXStructs::Rect resultBox = {0, 0, constants::RESULT_PLACEMENT_WIDTH, constants::RESULT_PLACEMENT_HEIGHT};
    // set up grid data
    GFXStructs::GridPlacementData placementData = _lcdServices.calculateGridDataCentered(resultBox, _lcdServices.getSpriteWidth(spriteEnum), _lcdServices.getSpriteHeight(spriteEnum), _amountRolled, page);
    // place dice
    _lcdServices.populateResultBoxCentered(page, _rolls.data(), _rolledDieTypeIndex.data(), resultBox, placementData, spriteEnum, images);
}



// TOOOOOD padding top is not working...
// void RollModeGUI::populateResultBox(int page, int resultBoxX, int resultBoxY, int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, const uint16_t* const* imageWidths, const uint16_t* const* imageHeights, const uint16_t* const* images, int colGapSize, int rowGapSize)
// {
//     // set up char array to display. we must convert int to char array
//     char buffer[6];
//     // TESTING
//     _tft.fillRect(resultBoxX, resultBoxY, resultBoxWidth, resultBoxHeight, TFT_BLACK);
//     // TESTING
//     // since we are basing amount of dice we can place based in a row or col based on the padding, not the other way around, the calculations are a little different
//     // divide total area by width of sprite + gap size. The final die in row/col does NOT place the gap padding to its right like the others
//     // meaning there is one less "gap" then there are sprites being pushed per row/col. to ammend this, adding the gap size to width/height
//     // of the placement area effectivly cancels out one gap, giving us an accurate calculation of have many dice can properly fit with the given gap.
//     // sorry future me or other people that see this that find that explanation confusing, I'm not great at explaining the math in my head.
//     // but I'm sure you're smart soooooooooo you probably didn't really need the explanation anyway LOL.
//     int maxDicePerRow = (resultBoxWidth + colGapSize)/(sprite.width() + colGapSize);
//     int maxDicePerCol = (resultBoxHeight + rowGapSize)/(sprite.height() + rowGapSize);
//     // self explanitory (i need sleep)
//     int maxDiceOnPage = maxDicePerCol * maxDicePerRow;
//     // this is similar to the standard version to find the padding, we just need to find how many pixels are remaining
//     // after the previous equation an cut it in half.
//     int paddingLeft = ((resultBoxWidth + colGapSize)%(sprite.width() + colGapSize))/2;
//     int paddingTop = ((resultBoxHeight + rowGapSize)%(sprite.height() + rowGapSize))/2;
//     // TESTING
//     _tft.drawRect(resultBoxX, resultBoxY, resultBoxWidth, resultBoxHeight, TFT_WHITE);
//     int currentXPlacement = resultBoxX + paddingLeft;
//     int currentYPlacement = resultBoxY + paddingTop;
//     // shift rolls index based on page
//     int startIndex = page * (maxDiceOnPage);
//     //index of the first die outside of page
//     int endIndex = (page + 1) * maxDiceOnPage;
//     int index = 0;
//     // loop through each die we have (d20, d12, d10 etc)
//     for(int i = 0; i < _dieAmounts.size(); i++)
//     {
//         // we have to allign index and _dieAmounts to potray the correct die rolled
//         if (index + _dieAmounts[i] <= startIndex)
//         {
//             index += _dieAmounts[i];
//         }
//         else
//         {
//             // roll i dice j times
//             for(int j = 0; j < _dieAmounts[i]; j++)
//             {
//                 // we need to get I and j on the same page as _rolls or else it will print incorect dice images
//                 if(index >= startIndex)
//                 {
//                     // if we go over the bounds we have set, reset x and inc y, (if the next die will go over the box we've set)
//                     if (currentXPlacement + sprite.width() > resultBoxWidth + resultBoxX)
//                     {
//                         currentXPlacement = resultBoxX + paddingLeft;
//                         currentYPlacement += sprite.height() + rowGapSize;
//                     }
//                     snprintf(buffer, 6, "%u", _rolls[index]);
//                     pushDiceSprite(sprite, currentXPlacement, currentYPlacement, *imageWidths[i], *imageHeights[i], images[i], buffer);
//                     currentXPlacement += sprite.width() + colGapSize;
//                     delay(10);
//                 }
//                 index++;
//                 // once we've reached the end of the page
//                 if(index >= endIndex)
//                 {
//                     return;
//                 }
//             }
//         }
//     }
// }

// ===================================================================
// helper functions for interacting with tft
// ===================================================================
// TESTING MUST USE LCDSERVICES ONE FOR THIS
void RollModeGUI::drawBackground()
{
    // SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
    _tft.fillScreen(0x0);
    _tft.pushImage(6, 13, 302, 225, Images::image_paint_204_pixels);
    // ellipse 1
    // _tft.drawEllipse(330, 2, 59, 55, 0xFFFF);
    // string 2
    _tft.setTextColor(0xFFFF);
    _tft.setTextSize(2);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString("Roll Mode", 7, 6);

    // ellipse 1
    _tft.drawEllipse(326, 0, 45, 45, 0x2104);
    // ellipse 1
    _tft.drawEllipse(310, 13, 55, 55, 0x2104);
    // line 6
    _tft.drawLine(6, 227, 313, 227, 0xFFFF);
    // ellipse 1
    // _tft.drawEllipse(343, -13, 59, 55, 0xFFFF);
    // ButtonLeftSmall
    _tft.drawBitmap(84, 111, Images::image_ButtonLeftSmall_bits, 6, 10, 0xFFFF);
    // ButtonRightSmall
    _tft.drawBitmap(230, 112, Images::image_ButtonRightSmall_bits, 6, 10, 0xFFFF);
}
void RollModeGUI::drawResultBackground()
{
    // SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
    _tft.fillScreen(0x0);
    _tft.pushImage(6, 13, 302, 225, Images::image_paint_204_pixels);
    
}

// TOOOOOOD MAKE THIS DYNAMIC HOLY DONT HAVE TIME NOW LOOOOL
void RollModeGUI::modifyTotal()
{
    _tft.fillRect(0, 211, 330, 29, TFT_BLACK);
    char buffer[30];
    _tft.setTextFont(1);
    _tft.setTextSize(2);
    _tft.setTextColor(TFT_WHITE);
    if (_amountOfPages > 1)
    {
        _tft.setTextDatum(BL_DATUM);
        _tft.setCursor(0, constants::SCREEN_HEIGHT);
        snprintf(buffer, 25, "%d/%u", _currentResultPage + 1, _amountOfPages);
        _tft.drawString(buffer, 0, constants::SCREEN_HEIGHT - 3);
    }
    _tft.setTextDatum(BR_DATUM);
    if (_modifier != 0)
        snprintf(buffer, 30, "Total: %d (mod: %d)", _result + _modifier, _modifier);
    else
        snprintf(buffer, 30, "Total: %d", _result);
    _tft.drawString(buffer, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT - 3);
}
void RollModeGUI::eraseBottomDice()
{
    _tft.fillRect(
        constants::FIRST_LIL_DIE_X,
        constants::FIRST_LIL_DIE_Y,
        constants::SCREEN_WIDTH, // screen width
        Images::lilDieSpriteHeight,
        TFT_BLACK);
}