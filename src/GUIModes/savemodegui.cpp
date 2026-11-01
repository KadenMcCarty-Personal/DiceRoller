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

}

SaveModeGUI::SaveModeGUI(TFT_eSPI& tft, LCDServices &lcdServices) : _tft(tft), _lcdServices(lcdServices)
{
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
    _lcdServices.drawBackground();
    drawStaticItems();
    drawNonStaticItems();
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
        // ===========commented out for testing==========
    //     // lastly, check if the roll button has been pressed
    //     if (rollPressed)
    //     {
    //         screen = Screen::RESULTS;
    //         _currentResultPage = 0;

    //         //  if nothing selected, just add the current die we are on to selection and roll, then remove it from selection once rolled. 
    //         if (_amountSelected == 0)
    //         {

    //         }
    //         else
    //             rollSelected();
    //         enter();
    //     }
    //     break;
     }
    // case Screen::RESULTS:
    // {
    //     // if left or right is pressed while in result page we cycle through pages
    //     int pageDelta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
    //     // only change pages if the page actually would change
    //     if (pageDelta != 0 && _amountOfPages > 1 && _currentResultPage != constrain(_currentResultPage + pageDelta, 0, _amountOfPages - 1))
    //     {
    //         _currentResultPage = constrain(_currentResultPage + pageDelta, 0, _amountOfPages - 1);
    //         // after we change which page is selected, update the screen
    //         drawResultBackground();
    //         pushResults();
    //     }

    //     int addDelta = (subtractPressed ? -1 : 0) + (addPressed ? 1 : 0);
    //     if (addDelta != 0)
    //     {
    //         _modifier += addDelta;
    //         // TOOD I WANNA DO THIS NOW BUT DONT
    //         modifyTotal();
    //     }

    //     // lastly, check if the roll button has been pressed we switch back to selection screen
    //     if (rollPressed)
    //     {
    //         screen = Screen::SELECT;
    //         enter();
    //     }
    //     break;
    // }
     }
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
