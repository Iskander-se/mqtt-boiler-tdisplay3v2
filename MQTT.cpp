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

void cMQTT::callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, MQTT_PUBLISH_TOPIC_COM) == 0) {
    if (length == 0) return;

    int timeVal = 0;
    if (length == 1) {
      timeVal = payload[0] - '0';
    } else if (length == 2) {
      timeVal = (payload[0] - '0') * 10 + (payload[1] - '0');
    }

    if (timeVal >= 5 && timeVal <= 90) {
      state.timerMin = timeVal;
      state.timerSec = 0;
      Serial.printf("Core: Timer set to %d min\n", timeVal);
    } else if (payload[0] == '1') {
      if (state.timerMin < 1) state.timerMin = 20;
      state.heating = 1;
      state.timerSec = 60;
      Serial.println("Core: Heating ON");
    } else if (payload[0] == '0') {
      state.timerMin = 0;
      state.heating = 0;
      state.timerSec = 60;
      Serial.println("Core: Heating OFF");
    }




    return;
  }

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("] => ");
  Serial.println(message);
}

void cMQTT::mqttLoop(const BoilerStateData& stateData) {
  switch (mqttState) {
    case MqttState::IDLE:
      client.setServer(MQTT_HOST, MQTT_PORT);
      client.setCallback(cMQTT::callback);  // Вот эта привязка
      mqttRetries = 0;
      mqttState = MqttState::CONNECTING;
      break;

    case MqttState::CONNECTING:
      if (client.connected()) {
        mqttState = MqttState::CONNECTED;
        Serial.println("MqttState::CONNECTED");
        break;
      }

      if (client.connect(MQTT_BASE, MQTT_username, MQTT_password, MQTT_PUBLISH_willTopic, 0, true, MQTT_payloadNotAvailable, true)) {
        client.publish(MQTT_PUBLISH_willTopic, MQTT_payloadAvailable);
        client.subscribe(MQTT_PUBLISH_TOPIC_COM);
        Serial.println(MQTT_PUBLISH_willTopic);
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

        if (state.temp_tank != cachedState.temp_tank) client.publish(MQTT_PUBLISH_TOPIC_TEMP1, String(state.temp_tank, 1).c_str(), true);
        if (state.temp_solar != cachedState.temp_solar) client.publish(MQTT_PUBLISH_TOPIC_TEMP2, String(state.temp_solar, 1).c_str(), true);
        if (state.timerMin != cachedState.timerMin) client.publish(MQTT_PUBLISH_TOPIC_TIMER, String(state.timerMin).c_str(), true);

        if (state.start||state.heating != cachedState.heating) {
          client.publish(MQTT_PUBLISH_TOPIC_STATUS, String(state.heating).c_str(), true);
          client.publish(MQTT_PUBLISH_TOPIC_TIMER, String(state.timerMin).c_str(), true);
        }

        state.start = false;

        //Serial.print(MQTT_PUBLISH_TOPIC_STATUS);
        //Serial.println(String(stateData.heating).c_str());
        cachedState = stateData;
      }

      break;

    case MqttState::FAILED:
      // ждём события или таймера
      break;
  }
}