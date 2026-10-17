#pragma once

#include "../PeripheralDA/lcdinterface.h"

class AttackModeGUI
{
    public:
        explicit AttackModeGUI(LcdInterface& lcdInterface);
        void enter();
    private: 
        LcdInterface& _lcdInterface;
        uint8_t _navigation;
};