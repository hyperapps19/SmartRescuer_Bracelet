#include <config/display.conf.h>
#include <ui/statuses.h>
#include <Arduino.h>

#include "config/mqtt.conf.h"
#include "config/display.conf.h"

#include <mqtt/MqttHelperFunctions.h>

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <map>

DISPLAY_TYPE display; // !!! IMPORTANT !!! Must be GLOBAL or Exception will happen
HANDLER_TYPE* h;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi()
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  Serial.println("Connected to Wi-Fi.");
  //h->drawLoadStatus(STATUS_MQTT_CONN);
  //h->updateScreen();
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent)
{
 // h.get()->drawLoadStatus(STATUS_PULSE_SENSOR_INIT);
  //h.get()->updateScreen();
  sendMAX30102Data(mqttClient, 23, 232);
  std::map<u16, double> beaconDistances;
  beaconDistances.insert(std::make_pair(3, 1)); // First is beaconID, second is distance to this beacon
  beaconDistances.insert(std::make_pair(12, 576));
  sendDistancesFromBeacons(mqttClient, beaconDistances);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup()
{
  Serial.begin(2000000);
  display = UI_DISPLAY;
  INIT_DISPLAY(display);
  Serial.println("START");
  HANDLER_TYPE handler = HANDLER_TYPE(display);
  h = &handler;
  handler.drawLoadingScreen();
  handler.drawLoadStatus(STATUS_WIFI_CONN);
  handler.updateScreen();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
  //handler.drawLoadStatus(STATUS_PULSE_SENSOR_INIT);
  //handler.updateScreen();

  //handler.clearScreen();
  //handler.drawYesNoButtons();
  //handler.drawIsHelpHeeded();
  //handler.updateScreen();
}

void loop()
{
}
