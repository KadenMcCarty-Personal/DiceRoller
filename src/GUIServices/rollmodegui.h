#pragma once
#include <array>
#include <TFT_eSPI.h>
#include <iostream>
#include <charconv> // Required for std::to_chars

#include "../Data/images.h"
#include "../Services/functions.h"

class RollModeGUI
{
    public:
        explicit RollModeGUI(TFT_eSPI& tft);
        //function to call when switching to this mode. draws everything back to where it was when we left
        void enter();
        // function to call every loop, manges pretty much everything
        void update(bool leftPressed, bool rightPressed, bool addPressed, bool subtractPressed, bool rollPressed);
    private: 
        // draws the three selected dice using _navigation tell selection
        void pushSelectionDice();
        // erases then redraws all selected dice on the selection line
        void pushBottomDice();
        // updates only die that changed since prevDieAmounts. if one of the die amounts changed to or from 0 then update the whole thing. 
        void pushBottomDice(std::array<uint8_t, 6> prevDieAmounts);
        // draw a black rect over the bottom dice. useful for full resets
        void eraseBottomDice();
        // Draws the background and static objects 
        void drawBackground();
        // draws background for roll result screen
        void drawResultBackground();
        //fills the the roll result screen with dice an total
        void populateResultScreen();

        /// @brief  first fill with black then draw left die over that then push to screen to avoid flicker, parameter is the die we are drawing. uses a sprite. NOT Safe from going over or under array min/max.
        /// @param dieIndex index of the die we are pushing 
        /// @param x center x of where the sprite will be pushed
        /// @param y center y of where the sprite will be pushed
        void pushMedDie(uint8_t dieIndex, int32_t x, int32_t y);
        /// @brief  first fill with black then draw left die over that then push to screen to avoid flicker, parameter is the die we are drawing. uses a sprite. NOT Safe from going over or under array min/max.
        /// @param dieIndex index of the die we are pushing 
        /// @param x center x of where the sprite will be pushed
        /// @param y center y of where the sprite will be pushed
        void pushBigDie(uint8_t dieIndex, int32_t x, int32_t y);
        /// @brief  first fill with black then draw left die over that then push to screen to avoid flicker, parameter is the die we are drawing. uses a sprite. NOT Safe from going over or under array min/max.
        /// @param dieIndex index of the die we are pushing 
        /// @param x center x of where the sprite will be pushed
        /// @param y center y of where the sprite will be pushed
        void pushLilDie(uint8_t dieIndex, int32_t x, int32_t y);
        /// @brief same as pushlildie but you print a value onto it as well
        /// @param dieIndex 
        /// @param x 
        /// @param y 
        /// @param printValue 
        void pushLilDie(uint8_t dieIndex, int32_t x, int32_t y, char printValue[]);


        //---variables---
        TFT_eSPI& _tft;
        // we will use sprites for the main roll mode navigation to avoid flickering
        TFT_eSprite _bigDice;
        TFT_eSprite _medDice;
        TFT_eSprite _lilDice;
        // navigation through this page
        uint8_t _navigation;
        // amount of each die
        std::array<uint8_t, 6> _dieAmounts;
        // this is just the total amount of dice that are currently selected to be rolled. 
        // made into a member variable so we don't have to calculate it every time
        uint16_t _totalSelected;
        // signed int so can be +- (-128 to 127)
        int8_t _totalModifyer;
        // total rolled
        int16_t _total;
};