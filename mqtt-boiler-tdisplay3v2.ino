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
  delay(200);
  views.begin(&tft);
  views.setState(BoilerState::INITHW);
  coreInit();
  Serial.println("init");
}

sTimeer mainTime = { 0, 200 };   //GUI timer
sTimeer mqttTime = { 0, 2000 };  // x30 minute
int freeCounter = 0;
int mqttTimetick = 0;
bool inithw = false;

void loop() {
  unsigned long currentMillis = millis();

  coreButton();
  if (state.start || (currentMillis - mqttTime.previous >= mqttTime.period)) {
    mqttTime.previous = currentMillis;
    coreTick();
    MQTT.loop();
    inithw = true;
    Serial.printf("freeCounter = %-8d\n", freeCounter);
    freeCounter = 0;
  } else freeCounter++;

  if (currentMillis - mainTime.previous >= mainTime.period) {
    mainTime.previous = currentMillis;
    if (!inithw) return;
    if (state.heating) views.setState(BoilerState::HEATING);
    else if (state.timerSec) views.setState(BoilerState::SCREEN_WAKE);
    else views.setState(BoilerState::STANDBY);
    views.render(state);
  }
}
