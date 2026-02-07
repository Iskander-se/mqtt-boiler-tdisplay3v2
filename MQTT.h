#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include "core.h"

class cMQTT {
public:
  cMQTT();
  void loop();
  static void callback(char* topic, byte* payload, unsigned int length);

private:
  enum class WifiState : uint8_t { IDLE,
                                   CONNECTING,
                                   CONNECTED,
                                   FAILED };
  enum class MqttState : uint8_t { IDLE,
                                   CONNECTING,
                                   CONNECTED,
                                   FAILED };

  // Wi-Fi
  WifiState wifiState;
  uint32_t wifiStartMs;
  uint8_t wifiRetries;
  void wifiLoop();

  // MQTT
  MqttState mqttState;
  uint32_t mqttStartMs;
  uint8_t mqttRetries;
  void mqttLoop();

  // clients
  WiFiClient espClient;
  PubSubClient client;

  //core
  BoilerStateData cachedState{};

  // Callback
  //void (*messageCallback)(char* topic, byte* payload, unsigned int length);
  //void handleMessage(char* topic, byte* payload, unsigned int length);
};
extern cMQTT mqtt;