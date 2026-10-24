#pragma once
#include <array>
#include <TFT_eSPI.h>
#include <iostream>
#include <charconv> // Required for std::to_chars

#include "../Data/images.h"
#include "../Services/functions.h"
#include "../Data/gfxstructs.h"

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

        // draws background for roll result screen
        void drawResultBackground();
        /// @brief IMPORTANT NOTE this function currently handles the modification of _amountOfPages
        void pushResults();
        void pushOneDie();
        void modifyTotal();
        // MARKED FOR DEATH
        int dynamicGapCalculation(int length, int spriteLength);
        
        
        int getPageAmount(int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite);
        int getPageAmount(int resultBoxWidth, int resultBoxHeight, TFT_eSprite& sprite, int colGapSize, int rowGapSize);
        
        void centeredResults(int page, TFT_eSprite& sprite, const Images::Image* images);
        void centeredResultsOneDie(TFT_eSprite &sprite, const Images::Image *images);

        GFXStructs::GridPlacementData calculateGridDataCentered(GFXStructs::Rect box, int cellWidth, int cellHeight, int amountRolled, int page);
        GFXStructs::RowPlacementData calculateRowDataCentered(GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, int index);
        void populateResultBoxNewCentered(int page, uint8_t* rolls, GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, TFT_eSprite& sprite, const Images::Image* images);
        
        int getRolledDieType(int dieIndex);

        /// @brief Push a sprite to the screen with an image centered on it. 
        void pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, Images::Image image);
        /// @brief Push a sprite to the screen with an image centered on it. Includes a char array centered on sprite. 
        void pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, Images::Image image, char printValue[]);

        // Draws the background and static objects 
        void drawBackground();
        
        // consts that are used here in the header file
        static constexpr size_t NUM_DICE_TYPES = 6;
        static constexpr size_t MAX_ROLL = 500;
        // should be used in corelation with _dieAmounts. Shows what index each die type lives at. {20, 12, 10, 8, 6, 4} in this case.
        // must be changed if more dice are added
        static constexpr std::array<uint8_t, NUM_DICE_TYPES> DIE_TYPES = {20, 12, 10, 8, 6, 4};

        //---variables---

        // gfx library
        TFT_eSPI& _tft;

        // we will use sprites for the main roll mode navigation to avoid flickering
        TFT_eSprite _bigDice;
        TFT_eSprite _medDice;
        TFT_eSprite _lilDice;

        // navigation through this page
        uint8_t _navigation;
        // amount of each die to be rolled
        std::array<uint8_t, NUM_DICE_TYPES> _dieAmounts;

        // This array must be used in tandum with _dieAmounts and DIE_TYPES to know exactly what was rolled. 
        // example: _dieAmounts = {3, 1, 0, 0, 0, 1} 
        // _rolls would look something like {11, 16, 19, 7, 1}. using _dieAmounts we can tell that
        // the first 3 are d20s, the fourth is a d12, and the 5th is a d4. 
        std::array<uint8_t, MAX_ROLL> _rolls;
        // total result of all dice rolls, no modifier
        int16_t _result;
        //modifier to result
        int16_t _modifier;
        // total rolled
        int16_t _amountRolled;
        
        // total number of pages that can be displayed
        uint16_t _amountOfPages;
        uint16_t _currentResultPage;

        // TESTING (mayber) its late and I promised id finish TODAY!
        uint16_t _amountSelected;
};