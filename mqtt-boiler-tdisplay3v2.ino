#include "views.h"
#include "core.h"
#include "MQTT.h"

struct sTimeer {
  unsigned long previous;
  long period;
};


TFT_eSPI tft = TFT_eSPI(135, 240);
cMQTT mqtt;
Views views;
cCORE core;

void setup() {

  Serial.begin(115200);
  core.begin();
  mqtt.begin(&core);
  views.begin(&tft);
  views.setState(BoilerState::INITHW);
  core.state.setStatus("INIT...");
  Serial.println("init");
}

sTimeer mainTime = { 0, 200 };   //GUI timer
sTimeer mqttTime = { 0, 2000 };  // x30 minute
int freeCounter = 0;
int mqttTimetick = 0;

void loop() {
  unsigned long currentMillis = millis();

  //coreButton();
  if (currentMillis - mqttTime.previous >= mqttTime.period) {
    mqttTime.previous = currentMillis;
    core.tick();
    if (core.state.isReady()) mqtt.loop();
    Serial.printf("freeCounter = %-8d\n", freeCounter);
    freeCounter = 0;
  } else freeCounter++;

  if (currentMillis - mainTime.previous >= mainTime.period) {
    mainTime.previous = currentMillis;
    if (core.state.heating) views.setState(BoilerState::HEATING);
    else if (core.state.timerSec) views.setState(BoilerState::SCREEN_WAKE);
    else views.setState(BoilerState::STANDBY);
    views.render(core.state);
  }
}
