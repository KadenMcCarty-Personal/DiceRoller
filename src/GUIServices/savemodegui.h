#pragma once

#include "../PeripheralDA/lcdinterface.h"

class SaveModeGUI 
{
    public:
        explicit SaveModeGUI(LcdInterface& lcdInterface);
        void enter();
    private: 
        LcdInterface& _lcdInterface;
        uint8_t _navigation;
};