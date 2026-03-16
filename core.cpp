#include "core.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

cCORE::cCORE()
  : oneWire(ONE_WIRE_BUS_PIN),
    ds(&oneWire) {
}

void cCORE::begin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(BUTTON_01_PIN, INPUT);
  pinMode(BUTTON_02_PIN, INPUT);

  ds.begin();
  ds.setWaitForConversion(false);
  ds.setResolution(TANK_ADDR, 10);
  ds.setResolution(SOLAR_ADDR, 10);
  ds.requestTemperatures();
}



void cCORE::tick() {

  float t_tank = ds.getTempC(TANK_ADDR);
  float t_solar = ds.getTempC(SOLAR_ADDR);

  if (state.phase == SystemPhase::READY) {
    state.temp_tank = (state.temp_tank * 4 + t_tank) / 5;
    state.temp_solar = (state.temp_solar * 4 + t_solar) / 5;
  } else if (state.phase == SystemPhase::SENSORS) {
    state.temp_tank = t_tank;
    state.temp_solar = t_solar;
    state.setStatus("OK", SystemPhase::READY);
  } else {
    state.setStatus("SENS", SystemPhase::SENSORS);
  }
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

void cCORE::handleButton() {
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