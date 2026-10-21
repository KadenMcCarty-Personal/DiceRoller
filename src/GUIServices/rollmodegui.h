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
        //this just sets backend arrays 'n stuff like _rolls
        void rollSelected();
        void resetRolls();

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
        /// @brief made to remove the need for the overload of  populateResultScreen because I was tired of changing both of them. 
        /// @param length 
        /// @param spriteLength 
        /// @return 
        int dynamicGapCalculation(int length, int spriteLength);
        // I'll keep you commented out for now but consider yourself MARKED FOR DEATH
        // void populateResultScreen(int page, int resultBoxX, int resultBoxY, int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, const uint16_t* const* imageWidths, const uint16_t* const* imageHeights, const uint16_t* const* images);
        void populateResultScreen(int page, int resultBoxX, int resultBoxY, int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, const uint16_t* const* imageWidths, const uint16_t* const* imageHeights, const uint16_t* const* images, int colGapSize, int rowGapSize);
        int getPageAmount(int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite);
        int getPageAmount(int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, int colGapSize, int rowGapSize);
        void populateResultScreenCentered(int page, int resultBoxX, int resultBoxY, int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, const uint16_t* const* imageWidths, const uint16_t* const* imageHeights, const uint16_t* const* images);
        /// @brief Push a sprite to the screen with an image centered on it. 
        /// @param sprite 
        /// @param x global placement on screen based on sprite's center point coordinates, NOT sprite's top left corner.
        /// @param y global placement on screen based on sprite's center point coordinates, NOT sprite's top left corner.
        /// @param imageWidth 
        /// @param imageHeight 
        /// @param image 
        void pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, int32_t imageWidth, int32_t imageHeight, const uint16_t *image);
        /// @brief Push a sprite to the screen with an image centered on it. Includes a char array centered on sprite. 
        /// @param sprite 
        /// @param x global placement on screen based on sprite's center point coordinates, NOT sprite's top left corner.
        /// @param y global placement on screen based on sprite's center point coordinates, NOT sprite's top left corner.
        /// @param imageWidth 
        /// @param imageHeight 
        /// @param image 
        /// @param printValue char array to display on sprite
        void pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, int32_t imageWidth, int32_t imageHeight, const uint16_t *image, char printValue[]);

        // consts that are used here in the header file
        static constexpr size_t NUM_DICE_TYPES = 6;
        static constexpr size_t MAX_ROLL = 500;
        // should be used in corelation with _dieAmounts. Shows what index each die type lives at. {20, 12, 10, 8, 6, 4} in this case.
        // must be changed if more dice are added
        static constexpr std::array<uint8_t, NUM_DICE_TYPES> DIE_TYPES = {20, 12, 10, 8, 6, 4};
        //---variables---
        TFT_eSPI& _tft;
        // we will use sprites for the main roll mode navigation to avoid flickering
        TFT_eSprite _bigDice;
        TFT_eSprite _medDice;
        TFT_eSprite _lilDice;
        // navigation through this page
        uint8_t _navigation;
        // amount of each die
        std::array<uint8_t, NUM_DICE_TYPES> _dieAmounts;
        // This array must be used in tandum with _dieAmounts and DIE_TYPES to know exactly what was rolled. 
        // example: _dieAmounts = {3, 1, 0, 0, 0, 1} 
        // _rolls would look something like {11, 16, 19, 7, 1}. using _dieAmounts we can tell that
        // the first 3 are d20s, the fourth is a d12, and the 5th is a d4. 
        std::array<uint8_t, MAX_ROLL> _rolls;
        // this is just the total amount of dice that are currently selected to be rolled. 
        // made into a member variable so we don't have to calculate it every time
        uint16_t _totalSelected;
        // signed int so can be +- (-128 to 127)
        int8_t _totalModifyer;
        // total rolled
        int16_t _total;
};