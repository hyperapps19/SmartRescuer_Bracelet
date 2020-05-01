#ifndef MQTT_HELPER_FUNCTIONS_H
#define MQTT_HELPER_FUNCTIONS_H

#include <array>
#include <mqtt/MqttPath.cpp>
#include <AsyncMqttClient.h>
#include <map>

#include "config/mqtt.conf.h"

bool sendData(AsyncMqttClient& mqttClient, String suffix, String data);
bool sendMAX30102Data(AsyncMqttClient& mqttClient, int32_t heartRate, int32_t SpO2);
bool sendDistancesFromBeacons(AsyncMqttClient &mqttClient, std::map<u16, double> distances);

#endif