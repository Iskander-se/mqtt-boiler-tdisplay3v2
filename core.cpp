#pragma once
#include "core.h"
#include <Arduino.h>

constexpr uint8_t BUTTON_01_PIN = 35;
constexpr uint8_t BUTTON_02_PIN = 0;

BoilerStateData state;
ButtonState buttons;

void coreInit() {
  state.temp_tank = 50.1;
  state.temp_solar = 30.2;
  state.timerMin = 0;
  state.timerSec = 0;
  state.heating = false;
  state.otaActive = false;

  pinMode(BUTTON_01_PIN, INPUT);
  pinMode(BUTTON_02_PIN, INPUT);
  buttons.btn1 = false;
  buttons.btn2 = false;
}

void coreTick() {
  //state.tBoiler = readBoilerTemp();
  //state.tOutside = readOutsideTemp();

  state.heating = (state.timerMin > 0);



  if (state.timerMin > 0 && state.timerSec < 1) {
    state.timerMin--;
    state.timerSec = 60;
  }
  if (state.timerSec > 0) state.timerSec -= 2;

  // OTA
  // state.otaActive = ...
}

void coreButton() {
  static uint32_t btn1_delay, btn2_delay;

  if (!digitalRead(BUTTON_01_PIN)) {
    if (millis() - btn1_delay > 500) {
      if (!state.heating) {
        state.timerMin = 1;
        state.timerSec = 0;
      }
      state.timerMin += 10;
      if (state.timerMin > 90) state.timerMin = 90;
      btn1_delay = millis();
    }
  }

  if (!digitalRead(BUTTON_02_PIN)) {
    if (millis() - btn2_delay > 500) {
      state.timerMin = 0;
      state.heating = 0;
      state.timerSec = 60;
      btn2_delay = millis();
    }
  }
}