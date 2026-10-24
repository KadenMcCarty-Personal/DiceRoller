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

RollModeGUI::RollModeGUI(TFT_eSPI &tft) : _tft(tft), _bigDice(&tft), _medDice(&tft), _lilDice(&tft)
{
    _bigDice.createSprite(Images::bigDieSpriteWidth, Images::bigDieSpriteHeight);
    _bigDice.setTextSize(3);
    _bigDice.setTextColor(TFT_WHITE);
    _bigDice.setTextFont(1);
    _bigDice.setTextDatum(CC_DATUM);

    // init the lil dice sprite, the text there will be different
    _lilDice.createSprite(Images::lilDieSpriteWidth, Images::lilDieSpriteHeight);
    _lilDice.setTextSize(1);
    _lilDice.setTextColor(TFT_WHITE);
    _lilDice.setTextFont(2);
    _lilDice.setTextDatum(CC_DATUM);

    _medDice.createSprite(Images::medDieSpriteWidth, Images::medDieSpriteHeight);
    _medDice.setTextSize(2);
    _medDice.setTextColor(TFT_WHITE);
    _medDice.setTextFont(1);
    _medDice.setTextDatum(CC_DATUM);

    _dieAmounts = {0, 0, 0, 0, 0, 0};
    _navigation = 0;
    _amountRolled = 0;
    _amountSelected = 0;
    _rolls = {};
}

enum class Screen
{
    SELECT,
    RESULTS
};
Screen screen = Screen::SELECT;

void RollModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
{
    switch (screen)
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
            screen = Screen::RESULTS;
            _currentResultPage = 0;

            // TESTING (maybe)
            
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
            screen = Screen::SELECT;
            enter();
        }
        break;
    }
    }
}
void RollModeGUI::enter()
{
    switch (screen)
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
    // keep track of where we are on _rolls
    int rollsIndex = 0;
    // loop through each die we have (d20, d12, d10 etc)
    for (int i = 0; i < _dieAmounts.size(); i++)
    {
        // roll i dice j times
        for (int j = 0; j < _dieAmounts[i]; j++)
        {
            // input roll
            _rolls[rollsIndex] = random(1, DIE_TYPES[i] + 1);
            _result += _rolls[rollsIndex];
            rollsIndex++;
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
    pushDiceSprite(_medDice, constants::LEFT_DIE_X, constants::LEFT_DIE_Y, Images::medDice[navToDec]);
    pushDiceSprite(_bigDice, constants::CENTER_DIE_X, constants::CENTER_DIE_Y, Images::bigDice[_navigation]);
    pushDiceSprite(_medDice, constants::RIGHT_DIE_X, constants::RIGHT_DIE_Y, Images::medDice[navToInc]);
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
            pushDiceSprite(
                _lilDice,
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
            pushDiceSprite(
                _lilDice,
                constants::FIRST_LIL_DIE_X + placement * constants::PIXELS_BETWEEN_LIL_BOTTOM_DICE,
                constants::FIRST_LIL_DIE_Y,
                Images::lilDice[i],
                buffer);
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
        TFT_BLACK);
}

void RollModeGUI::pushResults()
{
    // TOOD (maybe): make this dynamic, magic numbers are typically no go but I may make an exception, we shall see
    // TEMP FOR TESTING
    drawResultBackground();
    // TEMP FOR TESTING
    if (_amountRolled <= 3)
    {
        _amountOfPages = 1;
        centeredResults(0, _bigDice, Images::bigDice);
    }
    else if (_amountRolled <= 18)
    {
        _amountOfPages = 1;
        centeredResults(0, _medDice, Images::medDice);
    }
    else
    {
        _amountOfPages = getPageAmount(
            constants::RESULT_PLACEMENT_WIDTH,
            constants::RESULT_PLACEMENT_HEIGHT, _lilDice,
            0,
            0);
        centeredResults(_currentResultPage, _lilDice, Images::lilDice);
    }
    // maybe not this in future
    modifyTotal();
}

void RollModeGUI::pushOneDie()
{
    centeredResults(0, _bigDice, Images::bigDice);
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

// MAKRED FOR DEATH
int RollModeGUI::dynamicGapCalculation(int length, int spriteLength)
{
    // we find the remainder of how many times sprite goes into lengths which gives us the amount of pixels we have left to use after space
    // is taken. we divide that by the amount of sprites we are using (-1 since the last die in
    // the sequence does not need the padding) to get the space between sprites
    return (length % spriteLength) / (length / spriteLength - 1);
}

int RollModeGUI::getPageAmount(int resultBoxWidth, int resultBoxHeight, TFT_eSprite &sprite, int colGapSize, int rowGapSize)
{
    // formula ripped straight from populateResultBox, dividing the total number of dice
    int temp = ((resultBoxWidth + colGapSize) / (sprite.width() + colGapSize)) * ((resultBoxHeight + rowGapSize) / (sprite.height() + rowGapSize));
    // ngl I was NOT smart enough to come up with this formula so do not credit me. always rounds up
    return (_amountRolled + temp - 1) / temp;
}


void RollModeGUI::centeredResults(int page, TFT_eSprite &sprite, const Images::Image *images)
{
    // make the box that the results will go into
    GFXStructs::Rect resultBox = {0, 0, constants::RESULT_PLACEMENT_WIDTH, constants::RESULT_PLACEMENT_HEIGHT};
    // set up grid data
    GFXStructs::GridPlacementData placementData = calculateGridDataCentered(resultBox, sprite.width(), sprite.height(), _amountRolled, page);
    // place dice
    populateResultBoxNewCentered(page, _rolls.data(), resultBox, placementData, sprite, images);
}
void RollModeGUI::centeredResultsOneDie(TFT_eSprite &sprite, const Images::Image *images)
{
    // make the box that the results will go into
    GFXStructs::Rect resultBox = {0, 0, constants::RESULT_PLACEMENT_WIDTH, constants::RESULT_PLACEMENT_HEIGHT};
    // set up grid data
    GFXStructs::GridPlacementData placementData = calculateGridDataCentered(resultBox, sprite.width(), sprite.height(), 1, 0);
    // place dice
    populateResultBoxNewCentered(0, _rolls.data(), resultBox, placementData, sprite, images);
}
GFXStructs::RowPlacementData RollModeGUI::calculateRowDataCentered(GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, int index)
{
    // the last row will have fewer cells in it, if we have less than the max remaining on to put on the page, then we use that to see how many are in the row
    // we take the first index of the row we are currently on to calculate hence the (index/maxcols)*maxcols which gets us exactly that
    int cellsInRow = std::min(placementData.cellAmount - ((index / placementData.maxCellsInRow) * placementData.maxCellsInRow), placementData.maxCellsInRow);
    // find how many pixels all of the cells themselves take up and subtract it by the
    // total amount of pixels to find how many pixels of space we have remaining.
    // there is one more gap than there is dice to place in the row since we
    // are counting either side of the row as a gap because of how centering would look
    // meaning we divide the remaining pixels by the amount of gaps we need.
    int xGap = (box.width - (cellsInRow * placementData.cellWidth)) / (cellsInRow + 1);
    // find the amount of pixels we have remaining by multiplying the cell width + gap by the amount of
    // cells in the row then adding one more gap since there is one more gap than cells. divide that
    // by two to find how many of those remaining pixels go on the left side. lastly add a xgap to the
    // result so we don't have to do it in the walk.
    int paddingLeft = ((box.width - (((placementData.cellWidth + xGap) * cellsInRow) + xGap)) / 2) + xGap;

    return {cellsInRow, xGap, paddingLeft};
}
GFXStructs::GridPlacementData RollModeGUI::calculateGridDataCentered(GFXStructs::Rect box, int cellWidth, int cellHeight, int amountRolled, int page)
{
    int maxCellsPerRow = box.width / cellWidth;
    int maxCellsPerCol = box.height / cellHeight;
    int maxCellsOnPage = maxCellsPerCol * maxCellsPerRow;

    // the total amnt of rolls - the starting point of where we begin writting said dice gives the amount of dice past that point (jeez im bad at explaining)
    // but we can have no more than the max on the page so we take the min between those.
    // but if we try to reach a page starting beyond the amount we rolled then we will have a negative cell amount so we keep the minimum to zero to avoid that
    int cellAmount = std::max(std::min(amountRolled - (page * maxCellsOnPage), maxCellsOnPage), 0);
    // total divided by rows rounding up since we will always have one col unless there are 0 dice on screen
    int cellsInCol = Functions::divideRoundUp(cellAmount, maxCellsPerRow);

    // divide the amount of pixels remaining after counting the ones used from cells. divide that by the amount of gaps we need
    int yGap = (box.height - (cellsInCol * cellHeight)) / (cellsInCol + 1);
    // find the amount of pixels we have remaining by multiplying the cell width + gap by the amount of
    // cells in the row then adding one more gap since there is one more gap than cells. divide that
    // by two to find how many of those remaining pixels go on the top side. lastly add a ygap to the
    // result so we don't have to do it in the walk.
    int paddingTop = ((box.height - (((cellHeight + yGap) * cellsInCol) + yGap)) / 2) + yGap;

    return {cellWidth, cellHeight, maxCellsPerRow, maxCellsPerCol, maxCellsOnPage, cellsInCol, cellAmount, yGap, paddingTop};
}
void RollModeGUI::populateResultBoxNewCentered(int page, uint8_t *rolls, GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, TFT_eSprite &sprite, const Images::Image *images)
{
    // TESTING
    _tft.fillRect(box.x, box.y, box.width, box.height, TFT_BLACK);
    // TESTING
    _tft.drawRect(box.x, box.y, box.width, box.height, TFT_WHITE);

    // create buffer for writing to sprite
    char buffer[6];
    // set the offset for accessing the dice from array
    int indexOffset = page * (placementData.maxCells);
    // instantiate row placement data struct so we dont realocate memory every single time
    GFXStructs::RowPlacementData rowPlacemendData;
    for (int i = 0; i < placementData.cellAmount; i++)
    {
        // calculate number of die in row, padding,  and xgap needed by calling this function
        rowPlacemendData = calculateRowDataCentered(box, placementData, i);
        // add a base of the box x pos and padding left. shift over by the amount of dice on current row at index * dice width and gap between dice
        int x = ((placementData.cellWidth + rowPlacemendData.cellGap) * (i % placementData.maxCellsInRow)) + box.x + rowPlacemendData.paddingLeft;
        // pretty much the same as x
        int y = ((placementData.cellHeight + placementData.yGap) * (i / placementData.maxCellsInRow)) + box.y + placementData.paddingTop;

        // print on those lil dice
        snprintf(buffer, 6, "%u", rolls[i + indexOffset]);
        pushDiceSprite(sprite, x, y, images[getRolledDieType(i + indexOffset)], buffer);
    }
}
int RollModeGUI::getRolledDieType(int dieIndex)
{
    int currIndex = 0;
    for (int i = 0; i < NUM_DICE_TYPES; i++)
    {
        if (currIndex + _dieAmounts[i] > dieIndex)
        {
            return i;
        }
        currIndex += _dieAmounts[i];
    }
    return -1;
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
void RollModeGUI::drawBackground()
{
    // SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
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
    // SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
    _tft.fillScreen(0x0);
    _tft.drawBitmap(11, 15, Images::image_paint_4_bits, 280, 204, 0xFFFF);
    _tft.drawBitmap(22, 17, Images::image_paint_7_bits, 265, 154, 0xFFFF);
}

void RollModeGUI::pushDiceSprite(TFT_eSprite &sprite, int32_t x, int32_t y, Images::Image image)
{
    // reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - image.width) / 2,
        (sprite.height() - image.height) / 2,
        image.width,
        image.height,
        image.data);
    // push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place
        x,
        y);
}
void RollModeGUI::pushDiceSprite(TFT_eSprite &sprite, int32_t x, int32_t y, Images::Image image, char printValue[])
{
    // reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - image.width) / 2,
        (sprite.height() - image.height) / 2,
        image.width,
        image.height,
        image.data);
    sprite.drawString(printValue, sprite.width() / 2 + 1, sprite.height() / 2);
    // push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place
        x,
        y);
}

