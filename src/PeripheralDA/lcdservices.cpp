#include "lcdservices.h"


LCDServices::LCDServices(TFT_eSPI &tft) : _tft(tft), _bigDice(&tft), _medDice(&tft), _lilDice(&tft)
{
    // init the sprites
    _bigDice.createSprite(Images::bigDieSpriteWidth, Images::bigDieSpriteHeight);
    _bigDice.setTextSize(3);
    _bigDice.setTextColor(TFT_WHITE);
    _bigDice.setTextFont(1);
    _bigDice.setTextDatum(CC_DATUM);
    
    _medDice.createSprite(Images::medDieSpriteWidth, Images::medDieSpriteHeight);
    _medDice.setTextSize(2);
    _medDice.setTextColor(TFT_WHITE);
    _medDice.setTextFont(1);
    _medDice.setTextDatum(CC_DATUM);

    _lilDice.createSprite(Images::lilDieSpriteWidth, Images::lilDieSpriteHeight);
    _lilDice.setTextSize(1);
    _lilDice.setTextColor(TFT_WHITE);
    _lilDice.setTextFont(2);
    _lilDice.setTextDatum(CC_DATUM);
}

GFXStructs::RowPlacementData LCDServices::calculateRowDataCentered(GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, int index)
{
    // the last row will have fewer cells in it, if we have less than the max remaining on to put on the page, then we use that to see how many are in the row
    // we take the first index of the row we are currently on to calculate hence the (index/maxcols)*maxcols which gets us exactly that
    // find how many pixels all of the cells themselves take up and subtract it by the
    // total amount of pixels to find how many pixels of space we have remaining.
    // there is one more gap than there is dice to place in the row since we
    // are counting either side of the row as a gap because of how centering would look
    // meaning we divide the remaining pixels by the amount of gaps we need.
    // find the amount of pixels we have remaining by multiplying the cell width + gap by the amount of
    // cells in the row then adding one more gap since there is one more gap than cells. divide that
    // by two to find how many of those remaining pixels go on the left side. lastly add a xgap to the
    // result so we don't have to do it in the walk.
    int cellsInRow = std::min(placementData.cellAmount - ((index / placementData.maxCellsInRow) * placementData.maxCellsInRow), placementData.maxCellsInRow);
    int xGap = (box.width - (cellsInRow * placementData.cellWidth)) / (cellsInRow + 1);
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
void LCDServices::populateResultBoxCentered(int page, uint8_t *rolls, uint8_t *rollImageIndex, GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, Sprites spriteEnum, const Images::Image *images)
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
        pushDiceSprite(spriteEnum, x, y, images[rollImageIndex[i + indexOffset]], buffer);
    }
}
// TESTING COULD BE DONE BETTER
void LCDServices::populateResultBoxCenteredColorText(int page, int8_t *rolls, int8_t *rollImageIndex, GFXStructs::Rect box, GFXStructs::GridPlacementData placementData, Sprites spriteEnum, const Images::Image *images)
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
        switch(rollImageIndex[i + indexOffset])
        {
            case 0: 
                _sprites[static_cast<int>(spriteEnum)]->setTextColor(TFT_PINK);
                break;
            case 3:
                _sprites[static_cast<int>(spriteEnum)]->setTextColor(TFT_GOLD);
                break;
            default :
                _sprites[static_cast<int>(spriteEnum)]->setTextColor(TFT_WHITE);
                break;

        }
        snprintf(buffer, 6, "%d", rolls[i + indexOffset]);
        pushDiceSprite(spriteEnum, x, y, images[rollImageIndex[i + indexOffset]], buffer);
    }
    _sprites[static_cast<int>(spriteEnum)]->setTextColor(TFT_WHITE);
}

void LCDServices::pushDiceSprite(Sprites spriteEnum, int32_t x, int32_t y, Images::Image image)
{
    TFT_eSprite& sprite = *_sprites[static_cast<int>(spriteEnum)];
    // reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - image.width) / 2,
        (sprite.height() - image.height) / 2,
        image.width,
        image.height,
        image.data);
    // push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place
        x,
        y);
}
void LCDServices::pushDiceSprite(Sprites spriteEnum, int32_t x, int32_t y, Images::Image image, char printValue[])
{
    TFT_eSprite& sprite = *_sprites[static_cast<int>(spriteEnum)];
    // reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - image.width) / 2,
        (sprite.height() - image.height) / 2,
        image.width,
        image.height,
        image.data);
    sprite.drawString(printValue, sprite.width() / 2 + 1, sprite.height() / 2);
    // push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place
        x,
        y);
}
void LCDServices::pushDiceSpriteCentered(Sprites spriteEnum, int32_t x, int32_t y, Images::Image image, char printValue[])
{
    TFT_eSprite& sprite = *_sprites[static_cast<int>(spriteEnum)];
    // reset sprite
    sprite.fillSprite(TFT_BLACK);
    // center the image on the sprite
    sprite.pushImage(
        // caluculate where the image must be placed on the sprite so it is centered
        (sprite.width() - image.width) / 2,
        (sprite.height() - image.height) / 2,
        image.width,
        image.height,
        image.data);
    sprite.drawString(printValue, sprite.width() / 2 + 1, sprite.height() / 2);
    // push sprite to screen at x and y (from sprites top left corner)
    sprite.pushSprite(
        // place
        x - (sprite.width() / 2),
        y - (sprite.height() / 2));
}

void LCDServices::drawBackground()
{
    // SUBJECT TO CHANGE, FIRST ITERATION FOR TESTING AND STUFF
    _tft.fillScreen(TFT_BLACK);
    _tft.pushImage(6, 13, 302, 225, Images::image_paint_204_pixels);
    // ellipse 1
    // _tft.drawEllipse(330, 2, 59, 55, 0xFFFF);

    // ellipse 1
    _tft.drawEllipse(326, 0, 45, 45, 0x2104);
    // ellipse 1
    _tft.drawEllipse(310, 13, 55, 55, 0x2104);
    // ellipse 1
    // _tft.drawEllipse(343, -13, 59, 55, 0xFFFF);
    // ButtonLeftSmall

}

int LCDServices::getSpriteWidth(Sprites spriteEnum)
{
    return (*_sprites[static_cast<int>(spriteEnum)]).width();
}
int LCDServices::getSpriteHeight(Sprites spriteEnum)
{
    return (*_sprites[static_cast<int>(spriteEnum)]).height();
}
