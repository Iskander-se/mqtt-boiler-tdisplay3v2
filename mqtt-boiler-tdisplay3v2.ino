#include <TFT_eSPI.h>
#include "MQTT.h"

#define TFT_BL 4
TFT_eSPI tft = TFT_eSPI(135, 240);

struct sTimeer {
  unsigned long previous;
  long period;
};

struct {
  int countdown = 0;
  int curView = 0;
  int lastView = 0;
  int wait = 0;
  bool relay = false;
  bool up = false;
} Sheduler;

cMQTT MQTT;

void setup() {

  Serial.begin(115200);
  //MQTT.setCallback(mqttCallback);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  analogWrite(TFT_BL, 100);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawString("failed", 10, 41);
  tft.drawString("failed2", 10, 71);


  Serial.println("init");
}

sTimeer mainTime = { 0, 200 };
sTimeer mqttTime = { 0, 2000 };
sTimeer ShedulerTime = { 0, 60000 };  //60000
int freeCounter1 = 0;
int freeCounter2 = 0;
int mqttTimetick = 0;
int ShedulerTimetick = 0;

void loop() {
  unsigned long currentMillis = millis();
  freeCounter1++;
  freeCounter2++;

  if (currentMillis - ShedulerTime.previous >= ShedulerTime.period) {
    ShedulerTime.previous = currentMillis;
    ShedulerTimetick = freeCounter1;
    freeCounter1 = 0;
    //***
  }

  if (currentMillis - mqttTime.previous >= mqttTime.period) {
    mqttTime.previous = currentMillis;
    mqttTimetick = freeCounter2;
    freeCounter2 = 0;
    MQTT.loop();
    //***
  }

  if (currentMillis - mainTime.previous >= mainTime.period) {
    mainTime.previous = currentMillis;

    //tft.fillScreen(TFT_BLACK);
    tft.drawString("shed:               |", 10, 41);
    tft.drawString(String(ShedulerTimetick), 70, 41);
    tft.drawString("mqtt:               |", 10, 71);
    tft.drawString(String(mqttTimetick), 70, 71);
    //**

    /*
    if (OTA) {
      **
      ArduinoOTA.handle();
    }

    if (Sheduler.lastView != Sheduler.curView) {
      Sheduler.lastView = Sheduler.curView;
      tft.fillScreen(TFT_BLACK);
    }
*/
  }
}
