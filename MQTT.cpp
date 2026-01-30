#include "mqtt.h"
#include "config.h"

cMQTT::cMQTT(): wifiState(WifiState::IDLE), wifiStartMs(0), wifiRetries(0),    mqttState(MqttState::IDLE), mqttRetries(0),    client(espClient) {}



void cMQTT::loop() {
  wifiLoop();
  if (WiFi.status() == WL_CONNECTED) {
    mqttLoop();
  }
}

void cMQTT::wifiLoop() {
  switch (wifiState) {
    case WifiState::IDLE:
      Serial.println("IDLE=WiFi.begin");
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      wifiStartMs = millis();
      wifiRetries = 0;
      wifiState = WifiState::CONNECTING;

      break;

    case WifiState::CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        wifiState = WifiState::CONNECTED;
        Serial.println("WifiState::CONNECTED");
      } else if (millis() - wifiStartMs > 5000) {
        if (++wifiRetries > 3) {
          wifiState = WifiState::FAILED;
        } else {
          WiFi.disconnect();
          WiFi.begin(WIFI_SSID, WIFI_PASS);
          wifiStartMs = millis();
        }
      }
      break;

    case WifiState::CONNECTED:
    case WifiState::FAILED:
      break;
  }
}

void cMQTT::mqttLoop() {
  switch (mqttState) {
    case MqttState::IDLE:
      client.setServer(MQTT_HOST, MQTT_PORT);
      mqttRetries = 0;
      mqttState = MqttState::CONNECTING;
      break;

    case MqttState::CONNECTING:
      if (client.connected()) {
        mqttState = MqttState::CONNECTED;
        Serial.println("MqttState::CONNECTED");
        break;
      }

      if (client.connect(MQTT_clientName, MQTT_username, MQTT_password, MQTT_PUBLISH_willTopic, 0, true, MQTT_payloadNotAvailable, true)) {
        client.publish(MQTT_PUBLISH_willTopic, MQTT_payloadAvailable);
        client.subscribe(MQTT_PUBLISH_TOPIC_COM);
        client.subscribe(MQTT_PUBLISH_TOPIC_stOTA);

        mqttState = MqttState::CONNECTED;
      } else {
        if (++mqttRetries > 3)
          mqttState = MqttState::FAILED;
      }

      break;

    case MqttState::CONNECTED:
      client.loop();
      Serial.println("MqttState::CONNECTED&WORK");
      if (!client.connected()) {
        Serial.println("MqttState::CONNECTED         ==          LOST");
        mqttState = MqttState::CONNECTING;
      }

      break;

    case MqttState::FAILED:
      // ждём события или таймера
      break;
  }
}