#pragma once
#include <stdint.h>

// ==== Hardware ====
// constexpr uint8_t ONE_WIRE_BUSpin = 25;
// constexpr uint8_t RELAYpin = 5;
// constexpr uint8_t BUTTON01pin = 35;
// constexpr uint8_t BUTTON02pin = 0;

// ==== WiFi ====
constexpr char WIFI_SSID[] = "####";  //Iskander-AP    //"xxx"
constexpr char WIFI_PASS[] = "####"; //"xxx";

// ==== MQTT ====
constexpr char MQTT_HOST[] = "192.168.5.#";
constexpr uint16_t MQTT_PORT = 1883;

const char* MQTT_username = "####"; // login user to server MQTT
const char* MQTT_password = "###"; // password login to server MQTT

#define MQTT_BASE "mqtt_boiler"  // host name device ini

#define MQTT_PUBLISH_TOPIC_STATUS MQTT_BASE "/status"
#define MQTT_PUBLISH_willTopic MQTT_BASE "/avaliability"
#define MQTT_PUBLISH_TOPIC_COM MQTT_BASE "/set"
#define MQTT_PUBLISH_TOPIC_TEMP1 MQTT_BASE "/temp1"
#define MQTT_PUBLISH_TOPIC_TEMP2 MQTT_BASE "/temp2"

#define MQTT_payloadAvailable "OK"
#define MQTT_payloadNotAvailable "Fail"