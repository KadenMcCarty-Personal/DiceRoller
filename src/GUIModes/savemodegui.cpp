#include "savemodegui.h"

namespace constants
{
    // the max number that the nav variable can be before looping back to 0
    constexpr uint8_t MAX_NAV = 3;
    // these are the lines we use to show what option is selected. one line would have a consistant index throughout all four arrays. 
    //MUST MATCH WITH _NAVIGATION made into an array of structs so we can easily access them using nav as an index
    struct NavLine {int x, y, x2, y2;};
    // these are the values that we use to draw as we add and subtract from the various values on the screen 
    //MUST MATCH WITH _NAVIGATION made into an array of structs so we can easily access them using nav as an index
    struct ValueGFX {
        int eraseX, eraseY, eraseWidth, eraseHeight; uint16_t eraseColor; int valueCenterX, valueCenterY; uint16_t valueColor; int valueSize, valueFont;
    };
    // values and their mins and maxs. MUST MATCH WITH _NAVIGATION. made into an array of structs so we can easily access them using nav as an index
    struct ValueMinMax {int min, max;};
    
    // Anything that changes with nav and is tied to and can be modified or requires gfx
    struct ValueData {SaveModeGUI::Field valueField; ValueMinMax valueMinMax; NavLine navLine; ValueGFX valueGFX;};
    constexpr ValueData MODIFIER_DATA { 
        SaveModeGUI::Field::MODIFIER,// modifier
        {-99, 99},
        {0, 82, 78, 82},
        {46, 61, 38, 14, TFT_WHITE, 64, 69, TFT_BLACK, 2, 1} 
    };
    constexpr ValueData DIFFICULTY_DATA { 
        SaveModeGUI::Field::DIFFICULTY,// difficulty
        {0, 99},
        {0, 145, 78, 145},
        {46, 124, 38, 14, TFT_WHITE, 61, 132, TFT_BLACK, 2, 1}
    };
    constexpr ValueData ADV_DATA { 
        SaveModeGUI::Field::ADV,// adv
        {-1, 1},
        {0, 207, 78, 207},
        {18, 186, 46, 14, TFT_WHITE, 41, 194, TFT_BLACK, 2, 1}
    };
    constexpr ValueData DICE_AMOUNT_DATA { 
        SaveModeGUI::Field::DICEAMOUNT, // amount
        {1, 99},
        {138, 226, 289, 226},
        {191, 126, 46, 14, 0xF1E0, 214, 134, TFT_WHITE, 2, 1}
    };
    // anything that can use _navigation as an index is in order here MUST BE IN ORDER
    constexpr ValueData const* SELECT_DATA[] = {
        &MODIFIER_DATA,
        &DIFFICULTY_DATA,
        &ADV_DATA,
        &DICE_AMOUNT_DATA
    };


        // this is the box where we can put dice into in the result screen
        constexpr uint16_t RESULT_PLACEMENT_X = 0;
        constexpr uint16_t RESULT_PLACEMENT_Y = 0;
        constexpr uint16_t RESULT_PLACEMENT_WIDTH = 320;
        constexpr uint16_t RESULT_PLACEMENT_HEIGHT = 210;

        constexpr uint16_t SCREEN_WIDTH = 320;
        constexpr uint16_t SCREEN_HEIGHT = 240;
}

SaveModeGUI::SaveModeGUI(TFT_eSPI& tft, LCDServices &lcdServices) : _tft(tft), _lcdServices(lcdServices)
{
    _navigation = 0;

    _modifier = 0;
    _difficulty = 0;
    _adv = 0;
    _diceAmount = 1;
    _rolls = {};
    _rollsPassFailIndex = {};
    _screen = Screen::SELECT;
}
void SaveModeGUI::enter()
{
    switch (_screen)
    {
    case Screen::SELECT:
        _lcdServices.drawBackground();
        drawStaticItems();
        drawNonStaticItems();
        break;
    case Screen::RESULTS:
        _lcdServices.drawBackground();
        pushRolls();
        break;
    }

}
void SaveModeGUI::drawStaticItems()
{
    //TESTING
    // [BEGIN lopaka generated]

    // rect 90
    _tft.fillRect(0, 56, 71, 23, 0xFFFF);
    // rect 90
    _tft.fillRect(0, 119, 71, 23, 0xFFFF);
    // rect 90
    _tft.fillRect(0, 181, 71, 23, 0xFFFF);
    // string 92
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextColor(0x0);
    _tft.setTextSize(2);
    _tft.drawString("Mod:", 2, 61);
    // string 92
    _tft.setTextColor(0xFFFF);
    _tft.drawString("Check Mode", 6, 6);
    // string 92
    _tft.setTextColor(0x0);
    _tft.drawString("DC:", 8, 124);
    // ellipse 91
    _tft.fillEllipse(74, 130, 13, 11, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(74, 192, 13, 11, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(74, 67, 13, 11, 0xFFFF);
    // // string 92
    // _tft.drawString("Adv", 20, 186);
    // // string 93
    // _tft.drawString("22", 48, 61);
    // // string 93
    // _tft.drawString("45", 47, 124);
    // d20croppedfinal
    _tft.pushImage(139, 47, 150, 173, Images::image_d20croppedfinal_pixels);
    // // line 106
    // _tft.drawLine(0, 82, 78, 82, 0xFFFF);
    // [END lopaka generated]

}



void SaveModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
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
                //erase old nav line
                drawNav(TFT_BLACK);
                // change nav
                _navigation = Functions::wrapAround<int>(_navigation + navDelta, 0, constants::MAX_NAV);
                // draw new nav line
                drawNav(TFT_WHITE);
            }

            // change the amount of dice that will be rolled if add or subtract is pressed based on which is selected.
            // add one for + and subtract one for -
            // make sure we dont go into the negatives with constrain
            int addDelta = (subtractPressed ? -1 : 0) + (addPressed ? 1 : 0);
            if (addDelta != 0)
            {
                // use select data struct to see what value is being changed and their min/max
                *_selectValue[_navigation] = constrain(*_selectValue[_navigation] + addDelta, constants::SELECT_DATA[_navigation]->valueMinMax.min, constants::SELECT_DATA[_navigation]->valueMinMax.max);
                drawValueAtIndex(_navigation);
            }
            // lastly, check if the roll button has been pressed
            if (rollPressed)
            {
                _screen = Screen::RESULTS;
                _currentResultPage = 0;
                roll();
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
                _lcdServices.drawBackground();
                pushRolls();
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
void SaveModeGUI::roll()
{
    resetRolls();
    for (int i = 0; i < _diceAmount; i ++)
    {
        switch (_adv) {
            case -1 :
            _rolls[i] = std::min(random(1, 21), random(1, 21)) + _modifier;
            break;

            case 0 : 
            _rolls[i] = random(1, 21) + _modifier;
            break;

            case 1 :
            _rolls[i] = std::max(random(1, 21), random(1, 21)) + _modifier;
            break;

            default:
            _rolls[i] = 0;
            break;
        }
        // TESTING 0 = crit fail, 1 = fail, 2 = successs, 3 = nat 20
        _rollsPassFailIndex[i] = (_rolls[i] - _modifier == 1 ? 0 : _rolls[i] - _modifier == 20 ? 3 : _rolls[i] < _difficulty ? 1 : _rolls[i] >= _difficulty ? 2 : -1);
    }
}
void SaveModeGUI::resetRolls()
{
    std::fill_n(_rolls.begin(), MAX_ROLL, 0);
    std::fill_n(_rollsPassFailIndex.begin(), MAX_ROLL, 0);
}

void SaveModeGUI::pushRolls()
{
     // TOOD (maybe): make this dynamic, magic numbers are typically no go but I may make an exception, we shall see
    if (_diceAmount <= 3)
    {
        _amountOfPages = 1;
        centeredResults(0, LCDServices::Sprites::BIGDICE, Images::passFailBigDice);
    }
    else if (_diceAmount <= 18)
    {
        _amountOfPages = 1;
        centeredResults(0, LCDServices::Sprites::MEDDICE, Images::passFailMedDice);
    }
    else
    {
        _amountOfPages = getPageAmount(
            constants::RESULT_PLACEMENT_WIDTH,
            constants::RESULT_PLACEMENT_HEIGHT, LCDServices::Sprites::LILDICE,
            0,
            0);
        centeredResults(_currentResultPage, LCDServices::Sprites::LILDICE, Images::passFailLilDice);
    }
    drawResults();
}
// TOOOOOOD MAKE THIS DYNAMIC HOLY DONT HAVE TIME NOW LOOOOL
void SaveModeGUI::drawResults()
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
        snprintf(buffer, 30, "%d/%u", _currentResultPage + 1, _amountOfPages);
        _tft.drawString(buffer, 0, constants::SCREEN_HEIGHT - 3);
    }
    _tft.setTextDatum(BR_DATUM);

    if (_difficulty > 0)
    {
        int amntPassed = 0;
        for (int i = 0; i < _diceAmount; i++)
        {
            // 2 and 3 = passed
            amntPassed += (_rollsPassFailIndex[i] >= 2 ? 1 : 0);
        }
        snprintf(buffer, 30, "Passed: %d/%d", amntPassed, _diceAmount);
        _tft.drawString(buffer, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT - 3);
    }
}

void SaveModeGUI::centeredResults(int page, LCDServices::Sprites spriteEnum, const Images::Image *images)
{
    // make the box that the results will go into
    GFXStructs::Rect resultBox = {0, 0, constants::RESULT_PLACEMENT_WIDTH, constants::RESULT_PLACEMENT_HEIGHT};
    // set up grid data
    GFXStructs::GridPlacementData placementData = _lcdServices.calculateGridDataCentered(resultBox, _lcdServices.getSpriteWidth(spriteEnum), _lcdServices.getSpriteHeight(spriteEnum), _diceAmount, page);
    // place dice
    _lcdServices.populateResultBoxCenteredColorText(page, _rolls.data(), _rollsPassFailIndex.data(), resultBox, placementData, spriteEnum, images);
}

int SaveModeGUI::getPageAmount(int resultBoxWidth, int resultBoxHeight, LCDServices::Sprites spriteEnum, int colGapSize, int rowGapSize)
{
    // formula ripped straight from populateResultBox, dividing the total number of dice
    int temp = ((resultBoxWidth + colGapSize) / (_lcdServices.getSpriteWidth(spriteEnum) + colGapSize)) * ((resultBoxHeight + rowGapSize) / (_lcdServices.getSpriteHeight(spriteEnum) + rowGapSize));
    // ngl I was NOT smart enough to come up with this formula so do not credit me. always rounds up
    return (_diceAmount + temp - 1) / temp;
}

void SaveModeGUI::drawValueAtIndex(int i)
{
    // erase the previous number
    _tft.fillRect(
        constants::SELECT_DATA[i]->valueGFX.eraseX, 
        constants::SELECT_DATA[i]->valueGFX.eraseY, 
        constants::SELECT_DATA[i]->valueGFX.eraseWidth,
        constants::SELECT_DATA[i]->valueGFX.eraseHeight,
        constants::SELECT_DATA[i]->valueGFX.eraseColor
    );
    //set up text settings
    _tft.setTextColor(constants::SELECT_DATA[i]->valueGFX.valueColor);
    _tft.setTextDatum(CC_DATUM);
    _tft.setTextSize(constants::SELECT_DATA[i]->valueGFX.valueSize);
    _tft.setTextFont(constants::SELECT_DATA[i]->valueGFX.valueFont);
    
    // the formatting will be slightly different based on what field we are changing so add a switch case
    switch (constants::SELECT_DATA[i]->valueField)
    {
    case SaveModeGUI::Field::ADV:
        _tft.drawString(
            //depending on _adv value, print the state of adv as a string
            (_adv < 0 ? "dis" : _adv > 0 ? "adv" : _adv == 0 ? "flat" : "error"), 
            constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
            constants::SELECT_DATA[i]->valueGFX.valueCenterY
        );
        break;
    case SaveModeGUI::Field::DICEAMOUNT:
        char buffer[6];
        snprintf(buffer, sizeof(buffer), "x%d", *_selectValue[i]);
        _tft.drawString(
            buffer,
            constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
            constants::SELECT_DATA[i]->valueGFX.valueCenterY            
        );
        break;
    default:
        // everything else just draws the number
        _tft.drawNumber(
            *_selectValue[i], 
            constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
            constants::SELECT_DATA[i]->valueGFX.valueCenterY
        );
        break;
    }
}
void SaveModeGUI::drawNonStaticItems()
{
    for(int i = 0; i <= constants::MAX_NAV; i++)
    {
        drawValueAtIndex(i);
    }
    drawNav(TFT_WHITE);
}

void SaveModeGUI::drawNav(uint32_t color)
{
    _tft.drawLine(constants::SELECT_DATA[_navigation]->navLine.x, 
        constants::SELECT_DATA[_navigation]->navLine.y, 
        constants::SELECT_DATA[_navigation]->navLine.x2, 
        constants::SELECT_DATA[_navigation]->navLine.y2, 
        color);
}
