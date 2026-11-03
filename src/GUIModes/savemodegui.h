#pragma once

#include "../PeripheralDA/lcdservices.h"

#include <TFT_eSPI.h>


class SaveModeGUI 
{
    public:
        explicit SaveModeGUI(TFT_eSPI& tft, LCDServices &lcdServices);
        void enter();
        void update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed);

        // THIS ENUM AND _selectValue MUST ALLIGN INDEXS FOR _navigation TO BE USED AS AN INDEX
        enum class Field {MODIFIER, DIFFICULTY, ADV, DICEAMOUNT};
    private: 
        enum class Screen
        {
            SELECT,
            RESULTS
        };
        Screen _screen;

        void drawStaticItems();
        void drawNonStaticItems();
        void drawNav(uint32_t color);
        void drawValueAtIndex(int i);
        void roll();
        void resetRolls();
        void pushRolls();
        int getPageAmount(int resultBoxWidth, int resultBoxHeight, LCDServices::Sprites spriteEnum, int colGapSize, int rowGapSize);
        void centeredResults(int page, LCDServices::Sprites spriteEnum, const Images::Image *images);
        void drawResults();

        static constexpr size_t MAX_ROLL = 99;
        TFT_eSPI& _tft;
        LCDServices& _lcdServices;
        uint8_t _navigation;

        std::array<int8_t, MAX_ROLL> _rolls;
        std::array<int8_t, MAX_ROLL> _rollsPassFailIndex;
        
        int _modifier;
        int _difficulty;
        int _adv;
        int _diceAmount;
        
        int* _selectValue[4] = {&_modifier, &_difficulty, &_adv, &_diceAmount};



        // total number of pages that can be displayed
        uint16_t _amountOfPages;
        uint16_t _currentResultPage;
};