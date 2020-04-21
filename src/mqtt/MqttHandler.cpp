#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#include "config/mqtt.conf.h"

class MqttHandler
{
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
    connectToMqtt();
  }

  void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
  {
    mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    wifiReconnectTimer.once(2, std::bind(&MqttHandler::connectToWifi, this));
  }

  void connectToMqtt()
  {
    mqttClient.connect();
  }

  void onMqttConnect(bool sessionPresent)
  {
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

  void setup()
  {
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
};