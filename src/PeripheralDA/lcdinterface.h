#pragma once

#include <math.h>
#include <TFT_eSPI.h>

#include "../Data/images.h"


class LcdInterface
{
  public: 
    explicit LcdInterface(TFT_eSPI& tft);
    //init the screen
    void begin();
    // pushes image using same paramters as tft_espi
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data);
    // draws a black rect at the x and y with width w and height h
    void drawBlackRect(int32_t x, int32_t y, int32_t w, int32_t h);
    // SUBJECT TO CHANGE draws roll mode background
    void drawRollModeBackground();
  private: 
    TFT_eSPI& _tft;
};