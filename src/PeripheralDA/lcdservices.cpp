#include "lcdservices.h"


LCDServices::LCDServices(TFT_eSPI &tft) : _tft(tft), _bigDice(&tft), _medDice(&tft), _lilDice(&tft)
{
}

GFXStructs::RowPlacementData LCDServices::calculateRowDataCentered(GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, int index)
{
    // the last row will have fewer cells in it, if we have less than the max remaining on to put on the page, then we use that to see how many are in the row
    // we take the first index of the row we are currently on to calculate hence the (index/maxcols)*maxcols which gets us exactly that
    int cellsInRow = std::min(placementData.cellAmount - ((index / placementData.maxCellsInRow) * placementData.maxCellsInRow), placementData.maxCellsInRow);
    // find how many pixels all of the cells themselves take up and subtract it by the
    // total amount of pixels to find how many pixels of space we have remaining.
    // there is one more gap than there is dice to place in the row since we
    // are counting either side of the row as a gap because of how centering would look
    // meaning we divide the remaining pixels by the amount of gaps we need.
    int xGap = (box.width - (cellsInRow * placementData.cellWidth)) / (cellsInRow + 1);
    // find the amount of pixels we have remaining by multiplying the cell width + gap by the amount of
    // cells in the row then adding one more gap since there is one more gap than cells. divide that
    // by two to find how many of those remaining pixels go on the left side. lastly add a xgap to the
    // result so we don't have to do it in the walk.
    int paddingLeft = ((box.width - (((placementData.cellWidth + xGap) * cellsInRow) + xGap)) / 2) + xGap;

    return {cellsInRow, xGap, paddingLeft};
}
GFXStructs::GridPlacementData LCDServices::calculateGridDataCentered(GFXStructs::Rect box, int cellWidth, int cellHeight, int amountRolled, int page)
{
    int maxCellsPerRow = box.width / cellWidth;
    int maxCellsPerCol = box.height / cellHeight;
    int maxCellsOnPage = maxCellsPerCol * maxCellsPerRow;

    // the total amnt of rolls - the starting point of where we begin writting said dice gives the amount of dice past that point (jeez im bad at explaining)
    // but we can have no more than the max on the page so we take the min between those.
    // but if we try to reach a page starting beyond the amount we rolled then we will have a negative cell amount so we keep the minimum to zero to avoid that
    int cellAmount = std::max(std::min(amountRolled - (page * maxCellsOnPage), maxCellsOnPage), 0);
    // total divided by rows rounding up since we will always have one col unless there are 0 dice on screen
    int cellsInCol = Functions::divideRoundUp(cellAmount, maxCellsPerRow);

    // divide the amount of pixels remaining after counting the ones used from cells. divide that by the amount of gaps we need
    int yGap = (box.height - (cellsInCol * cellHeight)) / (cellsInCol + 1);
    // find the amount of pixels we have remaining by multiplying the cell width + gap by the amount of
    // cells in the row then adding one more gap since there is one more gap than cells. divide that
    // by two to find how many of those remaining pixels go on the top side. lastly add a ygap to the
    // result so we don't have to do it in the walk.
    int paddingTop = ((box.height - (((cellHeight + yGap) * cellsInCol) + yGap)) / 2) + yGap;

    return {cellWidth, cellHeight, maxCellsPerRow, maxCellsPerCol, maxCellsOnPage, cellsInCol, cellAmount, yGap, paddingTop};
}
void LCDServices::populateResultBoxCentered(int page, uint8_t *rolls, uint8_t *rollImageIndex, GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, TFT_eSprite &sprite, const Images::Image *images)
{
    // TESTING
    // _tft.fillRect(box.x, box.y, box.width, box.height, TFT_BLACK);
    // TESTING
    // _tft.drawRect(box.x, box.y, box.width, box.height, TFT_WHITE);

    // create buffer for writing to sprite
    char buffer[6];
    // set the offset for accessing the dice from array
    int indexOffset = page * (placementData.maxCells);
    // instantiate row placement data struct so we dont realocate memory every single time
    GFXStructs::RowPlacementData rowPlacemendData;
    for (int i = 0; i < placementData.cellAmount; i++)
    {
        // calculate number of die in row, padding,  and xgap needed by calling this function
        rowPlacemendData = calculateRowDataCentered(box, placementData, i);
        // add a base of the box x pos and padding left. shift over by the amount of dice on current row at index * dice width and gap between dice
        int x = ((placementData.cellWidth + rowPlacemendData.cellGap) * (i % placementData.maxCellsInRow)) + box.x + rowPlacemendData.paddingLeft;
        // pretty much the same as x
        int y = ((placementData.cellHeight + placementData.yGap) * (i / placementData.maxCellsInRow)) + box.y + placementData.paddingTop;

        // print on those lil dice
        snprintf(buffer, 6, "%u", rolls[i + indexOffset]);
        pushDiceSprite(sprite, x, y, images[rollImageIndex[i + indexOffset]], buffer);
    }
}