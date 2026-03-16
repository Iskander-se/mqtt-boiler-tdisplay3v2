#include "mqtt.h"
#include "config.h"

cMQTT::cMQTT()
  : wifiState(WifiState::IDLE), wifiStartMs(0), wifiRetries(0), mqttState(MqttState::IDLE), mqttRetries(0), client(espClient) {}

void cMQTT::begin(cCORE* corePtr) {
    _core = corePtr;
}

void cMQTT::loop() {
  if (!_core) return; // fix loose begin;
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
      _core->state.setStatus("WIFI...");
      break;

    case WifiState::CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        wifiState = WifiState::CONNECTED;
         _core->state.setStatus("WIFI.OK");
      } else if (millis() - wifiStartMs > 5000) {
        // if (++wifiRetries > 3) {
        //   wifiState = WifiState::FAILED;
        // } else {
        _core->state.setStatus("WIFI.ERR");
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

    // int timeVal = 0;
    // if (length == 1) {
    //   timeVal = payload[0] - '0';
    // } else if (length == 2) {
    //   timeVal = (payload[0] - '0') * 10 + (payload[1] - '0');
    // }
    // if (timeVal >= 5 && timeVal <= 90) {
    //   _core->state.timerMin = timeVal;
    //   _core->state.timerSec = 0;
    // } else if (payload[0] == '1') {
    //   if (_core->state.timerMin < 1) _core->state.timerMin = 20;
    //   _core->state.heating = 1;
    //   _core->state.timerSec = 60;
    // } else if (payload[0] == '0') {
    //   _core->state.timerMin = 0;
    //   _core->state.heating = 0;
    //   _core->state.timerSec = 60;
    // }
    // return;
  }
}

void cMQTT::mqttLoop() {
  switch (mqttState) {
    case MqttState::IDLE:
      client.setServer(MQTT_HOST, MQTT_PORT);
      client.setCallback(cMQTT::callback);
      mqttRetries = 0;
      mqttState = MqttState::CONNECTING;
      _core->state.setStatus("MQTT...");
      break;

    case MqttState::CONNECTING:
      if (client.connected()) {
        mqttState = MqttState::CONNECTED;
        break;
      }

      if (client.connect(MQTT_BASE, MQTT_username, MQTT_password, MQTT_PUBLISH_willTopic, 0, true, MQTT_payloadNotAvailable, true)) {
        client.publish(MQTT_PUBLISH_willTopic, MQTT_payloadAvailable, true);
        client.publish(MQTT_PUBLISH_TOPIC_STATUS, _core->state.heating ? "1" : "0", true);
        client.subscribe(MQTT_PUBLISH_TOPIC_COM);
        // client.subscribe(MQTT_PUBLISH_TOPIC_stOTA);
        _core->state.setStatus("MQTT.OK");
        mqttState = MqttState::CONNECTED;
      } else {
        if (mqttRetries > 30) {
          mqttState = MqttState::FAILED;
          wifiState = WifiState::IDLE;
        } else _core->state.setStatus("MQTT.ERR");
      }

      break;

    case MqttState::CONNECTED:
      client.loop();
      if (!client.connected()) {
        _core->state.setStatus("MQTT.LOST");
        mqttState = MqttState::CONNECTING;
      } else {
        char msg[16];
        float threshold = 0.1;

        if (abs(_core->state.temp_tank - cachedState.temp_tank) >= threshold) {
          cachedState.temp_tank=_core->state.temp_tank;
          dtostrf(_core->state.temp_tank, 4, 1, msg);
          client.publish(MQTT_PUBLISH_TOPIC_TEMP1, msg, true);
        }
        if (abs(_core->state.temp_solar - cachedState.temp_solar) >= threshold) {
          cachedState.temp_solar=_core->state.temp_solar;
          dtostrf(_core->state.temp_solar, 4, 1, msg);
          client.publish(MQTT_PUBLISH_TOPIC_TEMP2, msg, true);
        }


        if (_core->state.heating != cachedState.heating) {
          cachedState.heating = _core->state.heating;
          client.publish(MQTT_PUBLISH_TOPIC_STATUS, _core->state.heating ? "1" : "0", true);
          cachedState.timerMin = -1;
        }

        if (_core->state.timerMin != cachedState.timerMin) {
          cachedState.timerMin = _core->state.timerMin;
          itoa(_core->state.timerMin, msg, 10);
          client.publish(MQTT_PUBLISH_TOPIC_TIMER, msg, true);
        }
      }

      break;

    case MqttState::FAILED:
      mqttState = MqttState::IDLE;
      // TODO by timer
      break;
  }
}