#include "mqtt.h"
#include "config.h"

cMQTT::cMQTT()
  : wifiState(WifiState::IDLE), wifiStartMs(0), wifiRetries(0), mqttState(MqttState::IDLE), mqttRetries(0), client(espClient) {}



void cMQTT::loop(const BoilerStateData& stateData) {
  wifiLoop();
  if (WiFi.status() == WL_CONNECTED) {
    mqttLoop(stateData);
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

void cMQTT::mqttLoop(const BoilerStateData& stateData) {
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

      if (client.connect(MQTT_BASE, MQTT_username, MQTT_password, MQTT_PUBLISH_WILL_TOPIC, 0, true, MQTT_payloadNotAvailable, true)) {
        client.publish(MQTT_PUBLISH_WILL_TOPIC, MQTT_payloadAvailable);
        client.subscribe(MQTT_PUBLISH_TOPIC_COM);
        Serial.println(MQTT_PUBLISH_WILL_TOPIC);
        // client.subscribe(MQTT_PUBLISH_TOPIC_stOTA);

        mqttState = MqttState::CONNECTED;
      } else {
        if (++mqttRetries > 3)
          mqttState = MqttState::FAILED;
      }

      break;

    case MqttState::CONNECTED:
      client.loop();

      if (!client.connected()) {
        Serial.println("MqttState::CONNECTED         ==          LOST");
        mqttState = MqttState::CONNECTING;
      } else {

        //if (stateData.temp_tank != cachedState.temp_tank) changed = true;
        //else if (stateData.temp_solar != cachedState.temp_solar) changed = true;
        //else if (stateData.timerMin != cachedState.timerMin) changed = true;
        //else if (stateData.heating != cachedState.heating) changed = true;

        if (stateData.temp_tank != cachedState.temp_tank) client.publish("MQTT_PUBLISH_TOPIC_TEMP1", String(stateData.temp_tank, 1).c_str(), true);
        if (stateData.temp_solar != cachedState.temp_solar) client.publish("MQTT_PUBLISH_TOPIC_TEMP2", String(stateData.temp_solar, 1).c_str(), true);
        if (stateData.heating != cachedState.heating) client.publish("MQTT_PUBLISH_TOPIC_STATUS", String(stateData.heating, 1).c_str(), true);


        Serial.println(MQTT_PUBLISH_TOPIC_STATUS);
        cachedState = stateData;
      }

      break;

    case MqttState::FAILED:
      // ждём события или таймера
      break;
  }
}