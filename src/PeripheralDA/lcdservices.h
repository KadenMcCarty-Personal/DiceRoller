#pragma once
#include <array>
#include <TFT_eSPI.h>
// #include <iostream>
// #include <charconv> // Required for std::to_chars

#include "../Data/images.h"
#include "../Services/functions.h"
#include "../Data/gfxstructs.h"

class LCDServices
{
    public:
        explicit LCDServices(TFT_eSPI& tft);
        /// @brief Push a sprite to the screen with an image centered on it. 
        void pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, Images::Image image);
        /// @brief Push a sprite to the screen with an image centered on it. Includes a char array centered on sprite. 
        void pushDiceSprite(TFT_eSprite& sprite, int32_t x, int32_t y, Images::Image image, char printValue[]);


        GFXStructs::GridPlacementData calculateGridDataCentered(GFXStructs::Rect box, int cellWidth, int cellHeight, int amountRolled, int page);
        GFXStructs::RowPlacementData calculateRowDataCentered(GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, int index);
        void populateResultBoxCentered(int page, uint8_t* rolls, uint8_t *rollImageIndex, GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, TFT_eSprite& sprite, const Images::Image* images);
        
    private:
    TFT_eSPI _tft;

    // we will use sprites for the main roll mode navigation to avoid flickering
    TFT_eSprite _bigDice;
    TFT_eSprite _medDice;
    TFT_eSprite _lilDice;
};
