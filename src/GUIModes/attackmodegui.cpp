#include "attackmodegui.h"

namespace constants
{
    // the max number that the nav variable can be before looping back to 0
    constexpr uint8_t MAX_NAV = 9;

    struct ValueMinMax {int min, max;};

    struct ValueData {AttackModeGUI::Field valueField; ValueMinMax valueMinMax; GFXStructs::NavLine navLine; GFXStructs::ValueGFX valueGFX;};
    constexpr ValueData ATK_MOD_DATA { 
        AttackModeGUI::Field::ATK_MOD,
        {-99, 99},
        {0, 98, 51, 98},
        {9, 70, 34, 14, TFT_WHITE, 26, 78, TFT_BLACK, 2, 1} 
    };    
    constexpr ValueData AC_DATA { 
        AttackModeGUI::Field::AC,
        {0, 99},
        {0, 155, 51, 155},
        {9, 127, 34, 14, TFT_WHITE, 26, 135, TFT_BLACK, 2, 1} 
    };    
    constexpr ValueData ADV_DATA { 
        AttackModeGUI::Field::ADV,
        {-1, 1},
        {0, 212, 51, 212},
        {4, 178, 46, 14, TFT_WHITE, 26, 186, TFT_BLACK, 2, 1} 
    };
    constexpr ValueData D12_DATA { 
        AttackModeGUI::Field::D12,
        {0, 99},
        {68, 118, 119, 118},
        {0, 0, 0, 0, TFT_RED, 94, 89, TFT_WHITE, 0, 0} 
    };
    constexpr ValueData D10_DATA { 
        AttackModeGUI::Field::D10,
        {0, 99},
        {133, 113, 184, 113},
        {0, 0, 0, 0, TFT_PURPLE, 159, 89, TFT_WHITE, 0, 0} 
    };
    constexpr ValueData D8_DATA { 
        AttackModeGUI::Field::D8,
        {0, 99},
        {199, 120, 250, 120},
        {0, 0, 0, 0, TFT_BLUE, 225, 89, TFT_WHITE, 0, 0} 
    };
    constexpr ValueData D6_DATA { 
        AttackModeGUI::Field::D6,
        {0, 99},
        {101, 181, 152, 181},
        {0, 0, 0, 0, TFT_GREEN, 127, 153, TFT_WHITE, 0, 0} 
    };
    constexpr ValueData D4_DATA { 
        AttackModeGUI::Field::D6,
        {0, 99},
        {167, 178, 218, 178},
        {0, 0, 0, 0, TFT_YELLOW, 193, 153, TFT_WHITE, 0, 0} 
    };
    constexpr ValueData DMG_MOD_DATA { 
        AttackModeGUI::Field::DMG_MOD,
        {-99, 99},
        {271, 126, 319, 126},
        {278, 98, 34, 14, TFT_WHITE, 295, 106, TFT_BLACK, 2, 1} 
    };
    constexpr ValueData ATK_AMOUNT_DATA { 
        AttackModeGUI::Field::ATK_AMOUNT,
        {0, 99},
        {271, 183, 319, 183},
        {278, 156, 34, 14, TFT_WHITE, 295, 164, TFT_BLACK, 2, 1} 
    };
    constexpr ValueData const* SELECT_DATA[] = {
        &ATK_MOD_DATA,
        &AC_DATA,
        &ADV_DATA,
        &D12_DATA,
        &D10_DATA,
        &D8_DATA,
        &D6_DATA,
        &D4_DATA,
        &DMG_MOD_DATA,
        &ATK_AMOUNT_DATA
    };

    constexpr uint16_t RESULT_PLACEMENT_X = 0;
    constexpr uint16_t RESULT_PLACEMENT_Y = 0;
    constexpr uint16_t RESULT_PLACEMENT_WIDTH = 320;
    constexpr uint16_t RESULT_PLACEMENT_HEIGHT = 210;

    constexpr uint16_t SCREEN_WIDTH = 320;
    constexpr uint16_t SCREEN_HEIGHT = 240;
}

AttackModeGUI::AttackModeGUI(TFT_eSPI& tft, LCDServices& lcdServices) : _tft(tft), _lcdServices(lcdServices)
{
    _navigation = 0;

    _dieAmounts = {};
    _atkRolls = {};
    _dmg = {};

    _atkMod = 0;
    _ac = 0;
    _adv = 0;
    _dmgMod = 0;
    _atkAmount = 0;

    _screen = Screen::SELECT;
}
void AttackModeGUI::enter()
{
    switch (_screen)
    {
    case Screen::SELECT:
        _lcdServices.drawBackground();
        drawStaticItems();
        drawNonStaticItems();
        break;
    case Screen::RESULTS:
        // _lcdServices.drawBackground();
        // pushRolls();
        break;
    }
}
void AttackModeGUI::update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed)
{
    switch (_screen)
    {
    case Screen::SELECT:
    {
        int navDelta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
        if (navDelta != 0)
        {
            drawNav(TFT_BLACK);
            _navigation = Functions::wrapAround<int>(_navigation + navDelta, 0, constants::MAX_NAV);
            drawNav(TFT_WHITE);
        }

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
            // _screen = Screen::RESULTS;
            // _currentResultPage = 0;

            // //  if nothing selected, just add the current die we are on to selection and roll, then remove it from selection once rolled. 
            // if (_amountSelected == 0)
            // {
            //     _amountSelected++;
            //     _dieAmounts[_navigation]++;
            //     rollSelected();
            //     _amountSelected--;
            //     _dieAmounts[_navigation]--;
            // }
            // else
            //     rollSelected();
            // enter();
        }
        break;
    }
    case Screen::RESULTS:
    {
        // // if left or right is pressed while in result page we cycle through pages
        // int pageDelta = (leftPressed ? -1 : 0) + (rightPressed ? 1 : 0);
        // // only change pages if the page actually would change
        // if (pageDelta != 0 && _amountOfPages > 1 && _currentResultPage != constrain(_currentResultPage + pageDelta, 0, _amountOfPages - 1))
        // {
        //     _currentResultPage = constrain(_currentResultPage + pageDelta, 0, _amountOfPages - 1);
        //     // after we change which page is selected, update the screen
        //     drawResultBackground();
        //     pushResults();
        // }

        // int addDelta = (subtractPressed ? -1 : 0) + (addPressed ? 1 : 0);
        // if (addDelta != 0)
        // {
        //     _modifier += addDelta;
        //     // TOOD I WANNA DO THIS NOW BUT DONT
        //     modifyTotal();
        // }

        // // lastly, check if the roll button has been pressed we switch back to selection screen
        // if (rollPressed)
        // {
        //     _screen = Screen::SELECT;
        //     enter();
        // }
        // break;
    }
    }
}
void AttackModeGUI::drawStaticItems()
{ 
    int ellipse_1_color = 0x2124;
    _tft.fillScreen(0x0);
    // paint 204
    // _tft.pushImage(8, 8, 302, 225, image_paint_204_pixels);
    // ellipse 91
    _tft.fillEllipse(43, 72, 13, 22, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(276, 100, 13, 22, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(276, 157, 13, 22, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(102, 223, 10, 10, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(141, 223, 10, 10, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(178, 223, 10, 10, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(217, 223, 10, 10, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(43, 129, 13, 22, 0xFFFF);
    // ellipse 91
    _tft.fillEllipse(43, 186, 13, 22, 0xFFFF);
    // string 92
    _tft.setTextColor(0xFFFF);
    _tft.setTextSize(2);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString("Attack Mode", 9, 7);
    // croppedfinald12
    // _tft.pushImage(69, 68, 50, 52, image_croppedfinald12_pixels);
    // // croppedfinald10
    // _tft.pushImage(134, 72, 50, 43, image_croppedfinald10_pixels);
    // // croppedfinald8
    // _tft.pushImage(200, 66, 50, 57, image_croppedfinald8_pixels);
    // // croppedfinald6
    // _tft.pushImage(102, 127, 50, 50, image_croppedfinald6_pixels);
    // croppedfinald4
    // _tft.pushImage(168, 130, 50, 44, image_croppedfinald4_pixels);
    // rect 90
    _tft.fillRect(274, 78, 46, 45, 0xFFFF);
    // rect 90
    _tft.fillRect(274, 135, 46, 45, 0xFFFF);
    // rect 90
    _tft.fillRect(92, 220, 21, 20, 0xFFFF);
    // rect 90
    _tft.fillRect(131, 220, 21, 20, 0xFFFF);
    // rect 90
    _tft.fillRect(168, 220, 21, 20, 0xFFFF);
    // rect 90
    _tft.fillRect(207, 220, 21, 20, 0xFFFF);
    // rect 90
    _tft.fillRect(0, 50, 46, 45, 0xFFFF);
    // rect 90
    _tft.fillRect(0, 107, 46, 45, 0xFFFF);
    // rect 90
    _tft.fillRect(0, 164, 46, 45, 0xFFFF);
    // string 92
    _tft.setTextColor(0x0);
    _tft.setTextSize(1);
    _tft.drawString("Dmg Mod", 276, 85);
    // string 92
    _tft.drawString("Attacks", 276, 142);
    // string 93
    _tft.setTextSize(2);
    // string 92
    _tft.setTextSize(1);
    _tft.drawString("Atk Mod", 6, 57);
    // string 92
    _tft.drawString("AC", 20, 114);
    _tft.drawString("s1", 97, 219);
    // string 92
    _tft.drawString("s2", 136, 219);
    // string 92
    _tft.drawString("s3", 173, 219);
    // string 92
    _tft.drawString("s4", 212, 219);

    // line 106
    // _tft.drawLine(271, 126, 319, 126, 0xFFFF);
    // // line 106
    // _tft.drawLine(271, 183, 319, 183, 0xFFFF);
    // // line 106
    // _tft.drawLine(0, 99, 51, 99, 0xFFFF);
    // // line 106
    // _tft.drawLine(0, 155, 52, 155, 0xFFFF);
    // // line 106
    // _tft.drawLine(0, 212, 53, 212, 0xFFFF);
    // // line 106
    // _tft.drawLine(93, 209, 111, 209, 0xFFFF);
    // // line 106
    // _tft.drawLine(132, 209, 150, 209, 0xFFFF);
    // // line 106
    // _tft.drawLine(169, 209, 187, 209, 0xFFFF);
    // // line 106
    // _tft.drawLine(208, 209, 226, 209, 0xFFFF);
    // string 93
    // string 92
    // _tft.drawString("-99", 276, 99);
    // // string 93
    // _tft.drawString("-99", 276, 157);
    // // string 92
    // _tft.drawString("+", 100, 230);
    // // string 92
    // _tft.drawString("-", 139, 230);
    // // string 92
    // _tft.drawString("-", 176, 230);
    // // string 92
    // _tft.drawString("+", 215, 230);
    // _tft.setTextSize(2);
    // _tft.drawString("-99", 9, 71);
    // // string 93
    // _tft.drawString("-99", 9, 128);
    // // string 93
    // _tft.drawString("Flat", 5, 179);




    // ellipse 1
    _tft.drawEllipse(309, 7, 55, 55, ellipse_1_color);
    // ellipse 1
    _tft.drawEllipse(325, -6, 45, 45, 0x2104);
    // [END lopaka generated]

}
void AttackModeGUI::drawNonStaticItems()
{
    for(int i = 0; i <= constants::MAX_NAV; i++)
    {
        drawValueAtIndex(i);
    }
    drawNav(TFT_WHITE);
}

void AttackModeGUI::drawNav(uint32_t color)
{
    _tft.drawLine(
        constants::SELECT_DATA[_navigation]->navLine.x, 
        constants::SELECT_DATA[_navigation]->navLine.y, 
        constants::SELECT_DATA[_navigation]->navLine.x2, 
        constants::SELECT_DATA[_navigation]->navLine.y2, 
        color
    );
}
void AttackModeGUI::drawValueAtIndex(int i)
{    
    // the formatting will be slightly different based on what field we are changing so add a switch case
    switch (constants::SELECT_DATA[i]->valueField)
    {
        case AttackModeGUI::Field::ADV:
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
            _tft.drawString(
                //depending on _adv value, print the state of adv as a string
                (_adv < 0 ? "Dis" : _adv > 0 ? "Adv" : _adv == 0 ? "Flat" : "error"), 
                constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
                constants::SELECT_DATA[i]->valueGFX.valueCenterY
            );
            break;

        case AttackModeGUI::Field::D12:
        case AttackModeGUI::Field::D10:
        case AttackModeGUI::Field::D8:
        case AttackModeGUI::Field::D6:
        case AttackModeGUI::Field::D4:
        char buffer[6];
            if (*_selectValue[i] != 0) 
                snprintf(buffer, sizeof(buffer), "%d", *_selectValue[i]);
            else
            {
                buffer[0] = '\0';
            }
            _lcdServices.pushDiceSpriteCentered(
                LCDServices::Sprites::MEDDICE, 
                constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
                constants::SELECT_DATA[i]->valueGFX.valueCenterY,
                Images::medDice[i - 2], // -2 because indexs for images start at 0
                buffer
            );
            drawNav(TFT_WHITE);
            break;
            
        default:
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
            // everything else just draws the number
            _tft.drawNumber(
                *_selectValue[i], 
                constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
                constants::SELECT_DATA[i]->valueGFX.valueCenterY
            );
            break;
    }
}

