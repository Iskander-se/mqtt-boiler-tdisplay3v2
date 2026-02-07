#pragma once
#include "core.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>



// ==== Hardware ====
constexpr uint8_t ONE_WIRE_BUS = 25;
constexpr uint8_t RELAY_PIN = 5;
constexpr uint8_t BUTTON_01_PIN = 35;
constexpr uint8_t BUTTON_02_PIN = 0;

DeviceAddress TANK = { 0x28, 0x0C, 0xE8, 0xC7, 0x00, 0x00, 0x00, 0x38 };
DeviceAddress SOLAR = { 0x28, 0x74, 0x90, 0xC7, 0x00, 0x00, 0x00, 0x52 };


BoilerStateData state;
ButtonState buttons;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds(&oneWire);

void coreInit() {
  state.temp_tank = 0;
  state.temp_solar = 0;
  state.timerMin = 0;
  state.timerSec = 0;
  state.heating = false;
  state.otaActive = false;
  state.start = true;

  pinMode(BUTTON_01_PIN, INPUT);
  pinMode(BUTTON_02_PIN, INPUT);
  buttons.btn1 = false;
  buttons.btn2 = false;

  ds.begin();
  ds.setWaitForConversion(false);
  ds.setResolution(TANK, 11);
  ds.setResolution(SOLAR, 11);
  ds.requestTemperatures();
}

void coreTick() {

  state.temp_tank = ds.getTempC(TANK);
  state.temp_solar = ds.getTempC(SOLAR);
  ds.requestTemperatures();
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
        state.timerMin = 0;
        state.timerSec = 62;
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