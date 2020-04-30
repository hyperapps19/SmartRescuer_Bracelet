#ifndef MQTT_HANDLER
#define MQTT_HANDLER

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Arduino.h>

#include <ui/DisplayHandler.cpp>

#include "config/mqtt.conf.h"
#include "config/display.conf.h"
#include "ui/statuses.h"
#include <mqtt/MqttHelperFunctions.h>

class MqttHandler
{
public:
  AsyncMqttClient mqttClient;
  Ticker mqttReconnectTimer;

  WiFiEventHandler wifiConnectHandler;
  WiFiEventHandler wifiDisconnectHandler;
  Ticker wifiReconnectTimer;

  void connectToWifi()
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  void onWifiConnect(const WiFiEventStationModeGotIP &event)
  {
    Serial.println("WIF conn1");
    // dHandler->drawLoadStatus(STATUS_MQTT_CONN);
    // dHandler->updateScreen();
    Serial.println("WIF conn2");
    connectToMqtt();
  }

  void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
  {
    mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    wifiReconnectTimer.once(2, std::bind(&MqttHandler::connectToWifi, this));
  }

  void connectToMqtt()
  {
    Serial.println("conncecting mqqttt");
    mqttClient.connect();
  }

  void onMqttConnect(bool sessionPresent)
  {
    sendMAX30102Data(mqttClient, 23, 99);
    Serial.println("mqtt CONN");
  }

  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
  {
    if (WiFi.isConnected())
    {
      mqttReconnectTimer.once(2, std::bind(&MqttHandler::connectToMqtt, this));
    }
  }

  void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
  {
  }

  MqttHandler(HANDLER_TYPE *dHandler)
  {
    this->dHandler = dHandler;
    wifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&MqttHandler::onWifiConnect, this, std::placeholders::_1));
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(std::bind(&MqttHandler::onWifiDisconnect, this, std::placeholders::_1));
    mqttClient.onConnect(std::bind(&MqttHandler::onMqttConnect, this, std::placeholders::_1));
    mqttClient.onDisconnect(std::bind(&MqttHandler::onMqttDisconnect, this, std::placeholders::_1));
    mqttClient.onMessage(std::bind(&MqttHandler::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
#ifdef MQTT_NEEDS_AUTHENTIFICATION
    mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);
#endif
    connectToWifi();
  }

private:
  HANDLER_TYPE *dHandler;
};
#endif