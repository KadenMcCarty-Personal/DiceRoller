#pragma once
#include <array>

#include "../PeripheralDA/lcdinterface.h"
#include "../Data/images.h"
#include "../Services/functions.h"

class RollModeGUI
{
    public:
        explicit RollModeGUI(LcdInterface& lcdInterface);
        //function to call when switching to this mode. draws everything back to where it was when we left
        void enter();
        // function to call every loop, manges pretty much everything
        void update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed);
        // call this function to update the dice images in accordance with the _navigation variable we need to know where we are navigating from and 
        //where we are navigating to. we get where to from _navigation
        void navigate(uint8_t navFrom);
        // uses nav from parameter to draw three perfectly sized black squares to "erase" the three dice that are selected
        void eraseSelectionDice(uint8_t navFrom);
        // draws the three selected dice using _navigation tell selection
        void drawSelectionDice();
    private: 
        LcdInterface& _lcdInterface;
        // navigation through this page
        uint16_t _navigation;
        // amount of each die
        std::array<uint8_t, 6> _dieAmounts;
        // signed int so can be +- (-128 to 127)
        int8_t _totalModifyer;
        // total rolled
        int16_t _total;
};