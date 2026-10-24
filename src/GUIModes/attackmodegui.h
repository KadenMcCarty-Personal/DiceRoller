#pragma once

#include <TFT_eSPI.h>

class AttackModeGUI 
{
    public:
        explicit AttackModeGUI(TFT_eSPI& tft);
        void enter();
        void update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed);
    private: 
        TFT_eSPI& _tft;
        uint8_t _navigation;
};