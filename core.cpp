#include "core.h"
#include "config.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>





DeviceAddress TANK_ADDR = { 0x28, 0x0C, 0xE8, 0xC7, 0x00, 0x00, 0x00, 0x38 };
DeviceAddress SOLAR_ADDR = { 0x28, 0x74, 0x90, 0xC7, 0x00, 0x00, 0x00, 0x52 };

cCORE::cCORE(uint8_t owPin, uint8_t rPin, uint8_t b1Pin, uint8_t b2Pin)
  : _relayPin(RELAYpin), _btn01Pin(BUTTON01pin), _btn02Pin(BUTTON02pin), _oneWire(ONE_WIRE_BUSpin), _ds(&_oneWire) {}

void cCORE::begin() {
  pinMode(_relayPin, OUTPUT);
  digitalWrite(_relayPin, LOW);
  pinMode(BUTTON01pin, INPUT);
  pinMode(BUTTON02pin, INPUT);

  _ds.begin();
  _ds.setWaitForConversion(false);
  _ds.setResolution(TANK_ADDR, 10);
  _ds.setResolution(SOLAR_ADDR, 10);
  _ds.requestTemperatures();
}



void cCORE::ctick() {

  if (state.start) {
    state.temp_tank = ds.getTempC(TANK);
    state.temp_solar = ds.getTempC(SOLAR);
  } else {
    state.temp_tank = (state.temp_tank * 4 + ds.getTempC(TANK)) / 5;
    state.temp_solar = (state.temp_solar * 4 + ds.getTempC(SOLAR)) / 5;
  }

  ds.requestTemperatures();
  state.heating = (state.timerMin > 0);



  if (state.timerMin > 0 && state.timerSec < 1) {
    state.timerMin--;
    state.timerSec = 60;
  }
  if (state.timerSec > 0) state.timerSec -= 2;
  state.start = false;
  // OTA
  // state.otaActive = ...
}

void cCORE::handleButton() {
  static uint32_t btn1_delay, btn2_delay;

  if (!digitalRead(BUTTON_01_PIN)) {
    if (millis() - btn1_delay > 500) {
      state.start = true;
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