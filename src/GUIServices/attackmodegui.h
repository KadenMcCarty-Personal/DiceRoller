#pragma once

#include <TFT_eSPI.h>

class AttackModeGUI 
{
    public:
        explicit AttackModeGUI(TFT_eSPI& tft);
        void enter();
    private: 
        TFT_eSPI& _tft;
        uint8_t _navigation;
};