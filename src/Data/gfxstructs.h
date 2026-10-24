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
}