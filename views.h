#pragma once
#include <stdint.h>
#include <TFT_eSPI.h>
#include "Images.h"
#include "core.h"

#define TFT_BL 4
#define BL_STANDBY 110
#define BL_ACTIVE 250
#define BL_STEP 2

enum class BoilerState : uint8_t { INITHW,
                                   STANDBY,
                                   HEATING,
                                   SCREEN_WAKE };

class Views {
public:
  void begin(TFT_eSPI* display);
  void setState(BoilerState st);

  void render(const BoilerStateData& state);

private:
  TFT_eSPI* tft = nullptr;
  char statusStr[10];
  struct UIState {
    BoilerState cur;
    BoilerState prev;
    uint8_t backlight;
    bool redrawFrame;
    bool redrawData;
  } ui;

  BoilerStateData cachedState{};

  void ViewBG();
};