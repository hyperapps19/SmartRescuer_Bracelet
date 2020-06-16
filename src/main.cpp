#include <Arduino.h>
#include <ButtonHandler.cpp>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "ui/statuses.h"
#include "config/display.conf.h"
#include "config/mqtt.conf.h"

#define LEFT_BTN D5
#define RIGHT_BTN D6

ButtonHandler left(LEFT_BTN);
ButtonHandler right(RIGHT_BTN);
const int trigPin = D7;

#define VERSION "1.0"

DISPLAY_TYPE display; // !!! IMPORTANT !!! Must be GLOBAL or Exception will happen
HANDLER_TYPE *handler;

WiFiClient espClient;
PubSubClient client(espClient);

#define DEBUGGING_ENABLED
#ifdef DEBUGGING_ENABLED
#define DEBUG(msg) Serial.println(msg);
#else
#define DEBUG(msg)
#endif

void Trigger_US()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  DEBUG();
  DEBUG(String("Connecting to ") + WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  DEBUG();
  DEBUG("WiFi connected");
  DEBUG("IP address: ");
  DEBUG(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
}

bool firstMqttConn = true;
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    DEBUG("Attempting to connect to MQTT server...");
    String clientId = "Bracelet-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      if (firstMqttConn)
      {
        firstMqttConn = false;

        handler->drawLoadStatus(STATUS_PULSE_SENSOR_INIT);
        handler->updateScreen();
      }
      DEBUG("Connected to MQTT server.");
    }
    else
    {
      DEBUG("failed, rc=");
      DEBUG(client.state());
      DEBUG("try again in 5 seconds");
      delay(5000);
    }
  }
}

void ICACHE_RAM_ATTR leftBtnCallback(void)
{
  Serial.println("L");
}

void ICACHE_RAM_ATTR rightBtnCallback(void)
{
  Serial.println("R");
}

void setup()
{
  pinMode(trigPin, OUTPUT);
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  left.setCallback(leftBtnCallback);
  right.setCallback(rightBtnCallback);
  Serial.println();
  Serial.println("SmartRescuer Project --- Bracelet Firmware v" + String(VERSION));
  Serial.println();

  display = UI_DISPLAY;
  INIT_DISPLAY(display);
  HANDLER_TYPE h = HANDLER_TYPE(display);
  handler = &h;
  handler->drawAlarmScreen();
  //handler->drawLoadingScreen();
  // handler->drawLoadStatus(STATUS_WIFI_CONN);
  handler->updateScreen();

  setup_wifi();

  //handler->drawLoadStatus(STATUS_MQTT_CONN);
  // handler->updateScreen();

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
    reconnect();
  client.loop();
}