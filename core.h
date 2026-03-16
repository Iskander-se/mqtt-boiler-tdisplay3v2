#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

enum class SystemPhase : uint8_t {
  BOOT,
  SENSORS,
  READY,
  ERROR
};

struct BoilerStateData {
  float temp_tank;
  float temp_solar;
  uint16_t timerMin = 0;
  uint16_t timerSec = 0;
  bool heating = false;
  bool otaActive = false;
  SystemPhase phase = SystemPhase::BOOT;
  char statusStr[10] = "INIT";

  void setStatus(const char* msg) {
    if (msg == nullptr) return;
    snprintf(statusStr, sizeof(statusStr), "%s", msg);
  }

  void setStatus(const char* msg, SystemPhase p) {
    if (msg) snprintf(statusStr, sizeof(statusStr), "%s", msg);
    phase = p;
  }

  bool isReady() const {
    return phase == SystemPhase::READY;
  }
  //bool operator!=(const BoilerStateData& other) const {
  //  return (temp_tank != other.temp_tank || heating != other.heating || timerSec != other.timerSec);
  //}
};


class cCORE {
public:
  cCORE();
  void begin();
  void tick();
  void handleButton();

  BoilerStateData state;

private:
  const uint8_t RELAY_PIN = 5;
  const uint8_t BUTTON_01_PIN = 35;
  const uint8_t BUTTON_02_PIN = 0;
  const uint8_t ONE_WIRE_BUS_PIN = 25;

  const DeviceAddress TANK_ADDR = { 0x28, 0x0C, 0xE8, 0xC7, 0x00, 0x00, 0x00, 0x38 };
  const DeviceAddress SOLAR_ADDR = { 0x28, 0x74, 0x90, 0xC7, 0x00, 0x00, 0x00, 0x52 };

  OneWire oneWire;
  DallasTemperature ds;

  struct ButtonState {
    bool btn1;
    bool btn2;
  } _btns;
};