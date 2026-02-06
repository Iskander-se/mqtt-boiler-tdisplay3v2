#include "views.h"
#include <Arduino.h>

void Views::begin(TFT_eSPI* display) {
  tft = display;
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 100);
  tft->init();
  tft->setRotation(1);
  tft->fillScreen(TFT_BLACK);

  ui.cur = BoilerState::STANDBY;
  ui.prev = ui.cur;
  ui.redrawFrame = true;
  ui.redrawData = true;

  ui.backlight = BL_STANDBY;
}

void Views::setState(BoilerState st) {
  if (ui.cur == st) return;
  ui.prev = ui.cur;
  ui.cur = st;
  ui.redrawFrame = true;
  ui.backlight = (st == BoilerState::STANDBY) ? BL_STANDBY : BL_ACTIVE;
  analogWrite(TFT_BL, ui.backlight);
}

void Views::render(const BoilerStateData& stateData) {
  if (!tft) return;
  bool changed = false;


  if (stateData.temp_tank != cachedState.temp_tank) changed = true;
  else if (stateData.temp_solar != cachedState.temp_solar) changed = true;
  else if (stateData.timerMin != cachedState.timerMin) changed = true;
  else if (stateData.heating != cachedState.heating) changed = true;

  if (changed) {
    cachedState = stateData;
    ui.redrawData = true;
  } else {
    ui.redrawData = false;
  }

  if (ui.redrawFrame) {
    tft->fillScreen(TFT_BLACK);
    ViewBG();  // as before
    switch (ui.cur) {
      case BoilerState::STANDBY:
      case BoilerState::SCREEN_WAKE:
        tft->drawXBitmap(5, 34, thermometer48, 48, 48, TFT_GOLD);
        tft->drawXBitmap(2, 98, sun36, 36, 36, TFT_ORANGE, TFT_BLACK);
        break;
      case BoilerState::HEATING:
        tft->drawXBitmap(74, 116, sun18, 18, 18, TFT_ORANGE);
        break;
    }
    ui.redrawFrame = false;
    ui.redrawData = true;
  }
  char buf[10];
  if (ui.redrawData) {
    switch (ui.cur) {
      case BoilerState::STANDBY:
      case BoilerState::SCREEN_WAKE:

        tft->setTextSize(7);
        tft->setTextColor(TFT_GOLD, TFT_BLACK);
        dtostrf(stateData.temp_tank, 4, 1, buf);
        tft->drawString(buf, 67, 36);
        tft->setTextSize(5);
        tft->setTextColor(TFT_ORANGE, TFT_BLACK);
        dtostrf(stateData.temp_solar, 4, 1, buf);
        tft->drawString(buf, 43, 98);
        break;
      case BoilerState::HEATING:
        int timer1 = stateData.timerMin;
        tft->setTextSize(3);
        tft->setTextColor(TFT_GOLD, TFT_BLACK);
        dtostrf(stateData.temp_tank, 4, 1, buf);
        tft->drawString(buf, 1, 114);
        tft->setTextColor(TFT_ORANGE, TFT_BLACK);
        dtostrf(stateData.temp_solar, 4, 1, buf);
        tft->drawString(buf, 92, 114);

        tft->setTextSize(8);
        tft->setTextColor(TFT_RED, TFT_BLACK);
        tft->setTextPadding(tft->textWidth(" 88.8min "));
        snprintf(buf, sizeof(buf), "%.1dmin", timer1);
        tft->drawString(buf, (timer1 > 9 ? 18 : 8), 36);
        break;
    }
    ui.redrawData = false;
  }



  ui.redrawFrame = false;
}

void Views::ViewBG() {
  tft->setTextSize(2);
  tft->setTextColor(TFT_NAVY, TFT_BLACK);
  tft->drawString("String", 1, 2);
  tft->setTextColor(TFT_SILVER, TFT_BLACK);
  tft->drawString("ON (+10min)", 105, 2);
  //bottom
  tft->setTextSize(3);
  tft->setTextColor(TFT_RED, TFT_BLACK);
  tft->drawString("STOP", 168, 114);
}