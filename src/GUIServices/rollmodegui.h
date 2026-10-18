#pragma once
#include <array>
#include <TFT_eSPI.h>

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
        // call this function to update the dice images in accordance with the _navigation variable we need to know where we are navigating from and 
        //where we are navigating to. we get where to from _navigation
        ///MARKED FOR DEATH
        void navigate();
        /// @brief uses nav from parameter to draw three perfectly sized black squares to "erase" the three dice that are selected MARKED FOR DEATH
        /// @param navFrom 
        void eraseSelectionDice(uint8_t navFrom);
        // draws the three selected dice using _navigation tell selection
        void pushSelectionDice();
        // Draws the background and static objects 
        void drawBackground();

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
        
        TFT_eSPI& _tft;
        // we will use sprites for the main roll mode navigation to avoid flickering
        TFT_eSprite _bigDice;
        TFT_eSprite _medDice;
        // navigation through this page
        uint8_t _navigation;
        // amount of each die
        std::array<uint8_t, 6> _dieAmounts;
        // signed int so can be +- (-128 to 127)
        int8_t _totalModifyer;
        // total rolled
        int16_t _total;
};