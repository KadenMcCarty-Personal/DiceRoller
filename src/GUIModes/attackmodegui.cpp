#include "attackmodegui.h"

namespace constants
{
    // the max number that the nav variable can be before looping back to 0
    constexpr uint8_t MAX_NAV = 13;

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
        {2, 178, 46, 14, TFT_WHITE, 26, 186, TFT_BLACK, 2, 1} 
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
        {1, 99},
        {271, 183, 319, 183},
        {278, 156, 34, 14, TFT_WHITE, 295, 164, TFT_BLACK, 2, 1} 
    };
    constexpr ValueData S1_DATA { 
        AttackModeGUI::Field::S1,
        {0, 1},
        {93, 209, 111, 209},
        {100, 231, 5, 5, TFT_WHITE, 102, 233, TFT_BLACK, 1, 1} 
    };
    constexpr ValueData S2_DATA { 
        AttackModeGUI::Field::S2,
        {0, 1},
        {132, 209, 150, 209},
        {139, 231, 5, 5, TFT_WHITE, 141, 233, TFT_BLACK, 1, 1} 
    };
    constexpr ValueData S3_DATA { 
        AttackModeGUI::Field::S3,
        {0, 1},
        {169, 209, 187, 209},
        {176, 231, 5, 5, TFT_WHITE, 178, 233, TFT_BLACK, 1, 1} 
    };constexpr ValueData S4_DATA { 
        AttackModeGUI::Field::S4,
        {0, 1},
        {208, 209, 226, 209},
        {215, 231, 5, 5, TFT_WHITE, 217, 233, TFT_BLACK, 1, 1} 
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
        &ATK_AMOUNT_DATA,
        &S1_DATA,
        &S2_DATA,
        &S3_DATA,
        &S4_DATA
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
    _hitMiss = {};
    _dmg = {};

    _atkMod = 0;
    _ac = 0;
    _adv = 0;
    _dmgMod = 0;
    _atkAmount = 1;
    _loadedSaves = {};

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
        _lcdServices.drawBackground();
        pushRolls();
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
            switch(constants::SELECT_DATA[_navigation]->valueField) {
                case AttackModeGUI::Field::S1:
                case AttackModeGUI::Field::S2:
                case AttackModeGUI::Field::S3:
                case AttackModeGUI::Field::S4:

                if(*_selectValue[_navigation] == 1 && addDelta == 1)
                {
                    loadPageState(_navigation - static_cast<int>(AttackModeGUI::Field::S1));
                    *_selectValue[_navigation] = constrain(*_selectValue[_navigation] + addDelta, constants::SELECT_DATA[_navigation]->valueMinMax.min, constants::SELECT_DATA[_navigation]->valueMinMax.max);
                    enter();
                }
                else if(*_selectValue[_navigation] == 0 && addDelta == 1)
                    savePageState(_navigation - static_cast<int>(AttackModeGUI::Field::S1));
                    *_selectValue[_navigation] = constrain(*_selectValue[_navigation] + addDelta, constants::SELECT_DATA[_navigation]->valueMinMax.min, constants::SELECT_DATA[_navigation]->valueMinMax.max);
                    drawValueAtIndex(_navigation);
                break;
            default:
                *_selectValue[_navigation] = constrain(*_selectValue[_navigation] + addDelta, constants::SELECT_DATA[_navigation]->valueMinMax.min, constants::SELECT_DATA[_navigation]->valueMinMax.max);
                drawValueAtIndex(_navigation);
                break;
            }
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

void AttackModeGUI::roll()
{
    resetRolls();
    for (int i = 0; i < _atkAmount; i ++)
    {
        switch (_adv) {
            case -1 :
            _atkRolls[i] = std::min(random(1, 21), random(1, 21)) + _atkMod;
            break;

            case 0 : 
            _atkRolls[i] = random(1, 21) + _atkMod;
            break;

            case 1 :
            _atkRolls[i] = std::max(random(1, 21), random(1, 21)) + _atkMod;
            break;

            default:
            _atkRolls[i] = 0;
            break;
        }
        // TESTING 0 = crit fail, 1 = fail, 2 = successs, 3 = nat 20
        _hitMiss[i] = (_atkRolls[i] - _atkMod == 1 ? 0 : _atkRolls[i] - _atkMod == 20 ? 3 : _atkRolls[i] < _ac ? 1 : _atkRolls[i] >= _ac ? 2 : -1);

        for (int k = 0; k < _dieAmounts.size(); k++)
        {
            // roll i dice j times
            for (int j = 0; j < _dieAmounts[k]; j++)
            {
                // input roll
                _dmg[i] += std::max(0, static_cast<int>(random(1, DIE_TYPES[k] + 1)));
                Serial.printf("roll: %d\n", _dmg[i]);
                //if we crit, roll same dice again
                if (_hitMiss[i] == 3)
                {
                    _dmg[i] += std::max(0, static_cast<int>(random(1, DIE_TYPES[k] + 1)));
                    Serial.printf("crit roll: %d\n", _dmg[i]);
                }
            }
        }
        // finally add the damage modifier once
        _dmg[i] += _dmgMod;
    }
}
void AttackModeGUI::resetRolls()
{
    std::fill_n(_atkRolls.begin(), MAX_ROLL, 0);
    std::fill_n(_hitMiss.begin(), MAX_ROLL, 0);
    std::fill_n(_dmg.begin(), MAX_ROLL, 0);
}

void AttackModeGUI::pushRolls()
{
     // TOOD (maybe): make this dynamic, magic numbers are typically no go but I may make an exception, we shall see
    if (_atkAmount <= 3)
    {
        _amountOfPages = 1;
        centeredResults(0, LCDServices::Sprites::BIGDICE, Images::passFailBigDice);
    }
    else if (_atkAmount <= 18)
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
void AttackModeGUI::drawResults()
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

    // TESTING
    // if (_ac > 0)
    // {
        int amntPassed = 0;
        int totalDmg = 0;
        for (int i = 0; i < _atkAmount; i++)
        {
            // 2 and 3 = passed
            if(_hitMiss[i] >= 2)
            {
                amntPassed++;
                totalDmg += _dmg[i];
            }
        }
        snprintf(buffer, 30, "Dmg: %d  Hits: %d/%d", totalDmg, amntPassed, _atkAmount);
        _tft.drawString(buffer, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT - 3);
    // }
}
void AttackModeGUI::centeredResults(int page, LCDServices::Sprites spriteEnum, const Images::Image *images)
{
    // make the box that the results will go into
    GFXStructs::Rect resultBox = {0, 0, constants::RESULT_PLACEMENT_WIDTH, constants::RESULT_PLACEMENT_HEIGHT};
    // set up grid data
    GFXStructs::GridPlacementData placementData = _lcdServices.calculateGridDataCentered(resultBox, _lcdServices.getSpriteWidth(spriteEnum), _lcdServices.getSpriteHeight(spriteEnum), _atkAmount, page);
    // place dice
    _lcdServices.populateResultBoxCenteredColorText(page, _atkRolls.data(), _dmg.data(), _hitMiss.data(), resultBox, placementData, spriteEnum, images);
}
int AttackModeGUI::getPageAmount(int resultBoxWidth, int resultBoxHeight, LCDServices::Sprites spriteEnum, int colGapSize, int rowGapSize)
{
    // formula ripped straight from populateResultBox, dividing the total number of dice
    int temp = ((resultBoxWidth + colGapSize) / (_lcdServices.getSpriteWidth(spriteEnum) + colGapSize)) * ((resultBoxHeight + rowGapSize) / (_lcdServices.getSpriteHeight(spriteEnum) + rowGapSize));
    // ngl I was NOT smart enough to come up with this formula so do not credit me. always rounds up
    return (_atkAmount + temp - 1) / temp;
}

void AttackModeGUI::savePageState(int slotIndex)
{
    _saveStates[slotIndex].atkMod = _atkMod;
    _saveStates[slotIndex].ac = _ac;
    _saveStates[slotIndex].adv = _adv;
    _saveStates[slotIndex].dieAmounts = _dieAmounts;
    _saveStates[slotIndex].dmgMod = _dmgMod;
    _saveStates[slotIndex].atkAmount = _atkAmount;
}
void AttackModeGUI::loadPageState(int slotIndex)
{
    _atkMod = _saveStates[slotIndex].atkMod;
    _ac = _saveStates[slotIndex].ac;
    _adv = _saveStates[slotIndex].adv;
    _dieAmounts = _saveStates[slotIndex].dieAmounts;
    _dmgMod = _saveStates[slotIndex].dmgMod;
    _atkAmount = _saveStates[slotIndex].atkAmount;
}

void AttackModeGUI::drawStaticItems()
{ 
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
    // // ellipse 1
    // _tft.drawEllipse(309, 7, 55, 55, ellipse_1_color);
    // // ellipse 1
    // _tft.drawEllipse(325, -6, 45, 45, 0x2104);
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
        case AttackModeGUI::Field::S1:
        case AttackModeGUI::Field::S2:
        case AttackModeGUI::Field::S3:
        case AttackModeGUI::Field::S4:
            // erase the previous number
            _tft.fillRect(
                constants::SELECT_DATA[i]->valueGFX.eraseX, 
                constants::SELECT_DATA[i]->valueGFX.eraseY, 
                constants::SELECT_DATA[i]->valueGFX.eraseWidth,
                constants::SELECT_DATA[i]->valueGFX.eraseHeight,
                constants::SELECT_DATA[i]->valueGFX.eraseColor
            );
            if(*_selectValue[i] != 0)
            {
                //set up text settings
                _tft.setTextColor(constants::SELECT_DATA[i]->valueGFX.valueColor);
                _tft.setTextDatum(CC_DATUM);
                _tft.setTextSize(constants::SELECT_DATA[i]->valueGFX.valueSize);
                _tft.setTextFont(constants::SELECT_DATA[i]->valueGFX.valueFont);
                // everything else just draws the number
                _tft.drawString(
                    "+", 
                    constants::SELECT_DATA[i]->valueGFX.valueCenterX, 
                    constants::SELECT_DATA[i]->valueGFX.valueCenterY
                );
            }
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

