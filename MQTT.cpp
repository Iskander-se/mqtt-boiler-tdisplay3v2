#include "mqtt.h"
#include "config.h"

cMQTT::cMQTT()
  : wifiState(WifiState::IDLE), wifiStartMs(0), wifiRetries(0), mqttState(MqttState::IDLE), mqttRetries(0), client(espClient) {}



void cMQTT::loop() {
  wifiLoop();
  if (WiFi.status() == WL_CONNECTED) {
    mqttLoop();
  }
}

void cMQTT::wifiLoop() {
  switch (wifiState) {
    case WifiState::IDLE:
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      wifiStartMs = millis();
      wifiRetries = 0;
      wifiState = WifiState::CONNECTING;
      snprintf(state.statusStr, sizeof(state.statusStr), "WIFI...");
      break;

    case WifiState::CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        wifiState = WifiState::CONNECTED;
        snprintf(state.statusStr, sizeof(state.statusStr), "WIFI.OK");
      } else if (millis() - wifiStartMs > 5000) {
        // if (++wifiRetries > 3) {
        //   wifiState = WifiState::FAILED;
        // } else {
        snprintf(state.statusStr, sizeof(state.statusStr), "WIFI.ERR");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        wifiStartMs = millis();
        // }
      }
      break;

    case WifiState::CONNECTED:
    case WifiState::FAILED:
      break;
  }
}

void cMQTT::callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, MQTT_PUBLISH_TOPIC_COM) == 0) {
    if (length == 0) return;

    int timeVal = 0;
    if (length == 1) {
      timeVal = payload[0] - '0';
    } else if (length == 2) {
      timeVal = (payload[0] - '0') * 10 + (payload[1] - '0');
    }
    //state.start = true;
    if (timeVal >= 5 && timeVal <= 90) {
      state.timerMin = timeVal;
      state.timerSec = 0;
    } else if (payload[0] == '1') {
      if (state.timerMin < 1) state.timerMin = 20;
      state.heating = 1;
      state.timerSec = 60;
    } else if (payload[0] == '0') {
      state.timerMin = 0;
      state.heating = 0;
      state.timerSec = 60;
    }
    return;
  }
}

void cMQTT::mqttLoop() {
  switch (mqttState) {
    case MqttState::IDLE:
      client.setServer(MQTT_HOST, MQTT_PORT);
      client.setCallback(cMQTT::callback);
      mqttRetries = 0;
      mqttState = MqttState::CONNECTING;
      snprintf(state.statusStr, sizeof(state.statusStr), "MQTT...");
      break;

    case MqttState::CONNECTING:
      if (client.connected()) {
        mqttState = MqttState::CONNECTED;
        break;
      }

      if (client.connect(MQTT_BASE, MQTT_username, MQTT_password, MQTT_PUBLISH_willTopic, 0, true, MQTT_payloadNotAvailable, true)) {
        client.publish(MQTT_PUBLISH_willTopic, MQTT_payloadAvailable, true);
        client.publish(MQTT_PUBLISH_TOPIC_STATUS, state.heating ? "1" : "0", true);
        client.subscribe(MQTT_PUBLISH_TOPIC_COM);
        // client.subscribe(MQTT_PUBLISH_TOPIC_stOTA);
        snprintf(state.statusStr, sizeof(state.statusStr), "MQTT.OK");
        mqttState = MqttState::CONNECTED;
      } else {
        if (mqttRetries > 30) {
          mqttState = MqttState::FAILED;
          wifiState = WifiState::IDLE;
        } else snprintf(state.statusStr, sizeof(state.statusStr), "MQTT.ERR");
      }

      break;

    case MqttState::CONNECTED:
      client.loop();
      if (!client.connected()) {
        snprintf(state.statusStr, sizeof(state.statusStr), "MQTT.LOST");
        mqttState = MqttState::CONNECTING;
      } else {
        char msg[16];

        if (state.temp_tank != cachedState.temp_tank) {
          dtostrf(state.temp_tank, 4, 1, msg);
          client.publish(MQTT_PUBLISH_TOPIC_TEMP1, msg, true);
        }
        if (state.temp_solar != cachedState.temp_solar) {
          dtostrf(state.temp_solar, 4, 1, msg);
          client.publish(MQTT_PUBLISH_TOPIC_TEMP2, msg, true);
        }


        if (state.start || state.heating != cachedState.heating) {
          client.publish(MQTT_PUBLISH_TOPIC_STATUS, state.heating ? "1" : "0", true);
          cachedState.timerMin = -1;
        }

        if (state.timerMin != cachedState.timerMin) {
          itoa(state.timerMin, msg, 10);
          client.publish(MQTT_PUBLISH_TOPIC_TIMER, msg, true);
        }
        cachedState = state;
      }

      break;

    case MqttState::FAILED:
      mqttState = MqttState::IDLE;
      // TODO by timer
      break;
  }
}