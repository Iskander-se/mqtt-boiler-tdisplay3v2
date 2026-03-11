#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class cCORE {
public:
  cCORE();
  void begin();
  void tick();
  void handleButton();

private:
  struct sdata {
    float temp_tank;
    float temp_solar;
    uint16_t timerMin = 0;
    uint16_t timerSec = 0;
    bool heating = false;
    bool otaActive = false;
    bool start = true;
  } _sdata;

  struct ButtonState {
    bool btn1;
    bool btn2;
  }_btns;

}