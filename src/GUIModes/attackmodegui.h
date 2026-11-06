#pragma once

#include "../PeripheralDA/lcdservices.h"

#include <TFT_eSPI.h>

class AttackModeGUI 
{
    public:
        explicit AttackModeGUI(TFT_eSPI& tft, LCDServices& lcdServices);
        void enter();
        void update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed);
        // THIS ENUM AND _selectValue MUST ALLIGN INDEXS FOR _navigation TO BE USED AS AN INDEX
        enum class Field {
            ATK_MOD, 
            AC, 
            ADV, 
            D12,
            D10,
            D8,
            D6, 
            D4,
            DMG_MOD,
            ATK_AMOUNT
        };

    private: 
        void drawStaticItems();
        void drawNonStaticItems();


        void drawNav(uint32_t color);
        void drawValueAtIndex(int i);


        TFT_eSPI& _tft;
        LCDServices& _lcdServices;

        enum class Screen
        {
            SELECT,
            RESULTS
        };
        Screen _screen;

        static constexpr size_t NUM_DICE_TYPES = 5;
        static constexpr size_t MAX_ROLL = 99;

        // ysed in conjunction with die amounts
        static constexpr std::array<uint8_t, NUM_DICE_TYPES> DIE_TYPES = {12, 10, 8, 6, 4};
        std::array<int, NUM_DICE_TYPES> _dieAmounts;

        std::array<int, MAX_ROLL> _atkRolls;
        std::array<int, MAX_ROLL> _dmg;

        uint8_t _navigation;

        int _atkMod;
        int _ac;
        int _adv;
        int _dmgMod;
        int _atkAmount;

        int* _selectValue[10] = {
            &_atkMod, 
            &_ac, 
            &_adv, 
            &_dieAmounts[0],
            &_dieAmounts[1],
            &_dieAmounts[2],
            &_dieAmounts[3],
            &_dieAmounts[4],
            &_dmgMod,
            &_atkAmount
        };
};