#include "views.h"
#include "core.h"
#include "MQTT.h"

#define TFT_BL 4

struct sTimeer {
  unsigned long previous;
  long period;
};


TFT_eSPI tft = TFT_eSPI(135, 240);
cMQTT MQTT;
Views views;

void setup() {

  Serial.begin(115200);

  views.begin(&tft);
  views.setState(BoilerState::SCREEN_WAKE);
  coreInit();
  Serial.println("init");
  Serial.println(state.timerSec);
}

sTimeer mainTime = { 0, 200 };   //GUI timer
sTimeer mqttTime = { 0, 2000 };  // x30 minute
int freeCounter = 0;
int mqttTimetick = 0;

void loop() {
  unsigned long currentMillis = millis();
  freeCounter++;
  coreButton();
  if (currentMillis - mqttTime.previous >= mqttTime.period) {
    mqttTime.previous = currentMillis;
    freeCounter = 0;
    coreTick();
    MQTT.loop(state);

    //***
  }

  if (currentMillis - mainTime.previous >= mainTime.period) {
    mainTime.previous = currentMillis;

    if (state.heating) views.setState(BoilerState::HEATING);
    else if (state.timerSec) views.setState(BoilerState::SCREEN_WAKE);
    else views.setState(BoilerState::STANDBY);

    views.render(state);
    freeCounter = 0;
  }
}
