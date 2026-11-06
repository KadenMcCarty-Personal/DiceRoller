#pragma once
#include <stdint.h>

namespace GFXStructs
{
    struct Rect{int x; int y; int width; int height;};
    /// @brief 
    struct GridPlacementData{
        int cellWidth; 
        int cellHeight; 

        int maxCellsInRow; 
        int maxCellsInCol; 
        int maxCells; 

        int rowAmount; 
        int cellAmount;

        int yGap; 
        int paddingTop; 
    };
    struct RowPlacementData{int cellAmount; int cellGap; int paddingLeft;};
    
    struct NavLine {int x, y, x2, y2;};
    // these are the values that we use to draw as we add and subtract from the various values on the screen 
    struct ValueGFX {
        int eraseX, eraseY, eraseWidth, eraseHeight; uint16_t eraseColor; int valueCenterX, valueCenterY; uint16_t valueColor; int valueSize, valueFont;
    };
}