#include <Arduino.h>
//#include <ButtonHandler.cpp>
#include <string>

#include <cstdio>
#include <ctime>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

#include "ui/statuses.h"
#include "config/display.conf.h"
#include "config/mqtt.conf.h"

#define LEFT_BTN D5
#define RIGHT_BTN D6
#define MAX30102_BUFFER_SIZE 100

uint32_t irBuffer[MAX30102_BUFFER_SIZE];  //infrared LED sensor data
uint32_t redBuffer[MAX30102_BUFFER_SIZE]; //red LED sensor data

int32_t spo2;          //SPO2 value
int8_t validSPO2;      //indicator to show if the SPO2 calculation is valid
int32_t heartRate;     //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

//ButtonHandler left(LEFT_BTN);
//ButtonHandler right(RIGHT_BTN);
const int trigPin = D7;

MAX30105 particleSensor;

#define VERSION "1.0"

DISPLAY_TYPE display; // !!! IMPORTANT !!! Must be GLOBAL or Exception will happen
ICACHE_RAM_ATTR HANDLER_TYPE *handler;

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

uint64_t currentInternalTime = 0ULL; // Microseconds
uint64_t previousMicros64Val = 0ULL;

bool timerStarted = false;

void updateTimeByMQTT(void)
{
  client.publish("sync_time", "");
}

#define TIMEZONE_SHIFT (+7) // hours from UTC
#define MILLISECONDS_IN_HOUR (60 * 60 * 1000)
const float tz_shift_milliseconds = TIMEZONE_SHIFT * MILLISECONDS_IN_HOUR;

#define SHIFT_MS_1970_2000 946684800000

uint8_t prevHours = 0U, prevMinutes = 0U;
void updateClockByMicros64Val(uint64_t m)
{
  time_t rawtime = ((m / 1000) - SHIFT_MS_1970_2000 + tz_shift_milliseconds) / 1000;
  struct tm *ptm = localtime(&rawtime);
  uint8_t hours = ptm->tm_hour;
  uint8_t minutes = ptm->tm_min;

  if ((prevHours != hours) || (prevMinutes != minutes))
  {
    handler->setClock(hours, minutes);
    handler->updateScreen();
  }
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

void print_uint64_t(uint64_t num)
{

  char rev[128];
  char *p = rev + 1;

  while (num > 0)
  {
    *p++ = '0' + (num % 10);
    num /= 10;
  }
  p--;
  /*Print the number which is now in reverse*/
  while (p > rev)
  {
    Serial.print(*p--);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, "time") == 0)
  {
    currentInternalTime = strtoull(std::string(reinterpret_cast<const char *>(payload), length).c_str(), NULL, 0);
    previousMicros64Val = micros64();
    updateClockByMicros64Val(currentInternalTime);
    timerStarted = true;
  }
}

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
      DEBUG("Connected to MQTT server.");
      client.subscribe("time");
      updateTimeByMQTT();
      DEBUG("Published request to update internal clock.");
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

bool alarm = false;
bool leftBtnPressed = false, rightBtnPressed = false;

//void ICACHE_RAM_ATTR leftBtnCallback(void) { leftBtnPressed = true; }
//void ICACHE_RAM_ATTR rightBtnCallback(void) { rightBtnPressed = true; }

void processMAX30102Data()
{
  if ((!validSPO2) || (spo2 < 80))
    spo2 = -1;
  if ((!validHeartRate) || (heartRate < 50) || (heartRate > 200))
    heartRate = -1;
  handler->setPulseAndSpO2Values(heartRate, spo2);
  handler->updateScreen();
}

void setup()
{
  pinMode(trigPin, OUTPUT);
  Serial.begin(115200);
  //left.setCallback(leftBtnCallback);
  //right.setCallback(rightBtnCallback);
  Serial.println();
  Serial.println("SmartRescuer Project --- Bracelet Firmware v" + String(VERSION));
  Serial.println();

  display = UI_DISPLAY;
  INIT_DISPLAY(display);
  handler = new HANDLER_TYPE(display);

  handler->drawLoadingScreen();
  handler->drawLoadStatus(STATUS_WIFI_CONN);
  handler->updateScreen();

  setup_wifi();

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);

  handler->drawLoadStatus(STATUS_PULSE_SENSOR_INIT);
  handler->updateScreen();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1)
      ;
  }

  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4;  //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;        //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100;   //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;    //Options: 69, 118, 215, 411
  int adcRange = 4096;     //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  delay(1000);
  handler->clearScreen();

  handler->drawMainScreen();
  handler->updateScreen();

  for (byte i = 0; i < MAX30102_BUFFER_SIZE; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check();                   //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    Serial.print(redBuffer[i]);
    Serial.print(" ");
    Serial.println(irBuffer[i]);
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, MAX30102_BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  processMAX30102Data();
}

void loop()
{
  //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
  for (byte i = 25; i < MAX30102_BUFFER_SIZE; i++)
  {
    redBuffer[i - 25] = redBuffer[i];
    irBuffer[i - 25] = irBuffer[i];
  }

  //take 25 sets of samples before calculating the heart rate.
  for (byte i = 75; i < MAX30102_BUFFER_SIZE; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check();                   //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); // We're finished with this sample so move to next sample
  }

  // After gathering 25 new samples recalculate HR and SP02
  maxim_heart_rate_and_oxygen_saturation(irBuffer, MAX30102_BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  processMAX30102Data();

  if (timerStarted)
  {
    uint64_t now = micros64();
    currentInternalTime += now - previousMicros64Val;
    previousMicros64Val = now;
    updateClockByMicros64Val(currentInternalTime);
  }

  if (!client.connected())
    reconnect();
  client.loop();
  /*bool mode = true;
  if (leftBtnPressed)
  {
    leftBtnPressed = false;
    rightBtnPressed = false;
    mode = HANDLER_TYPE::MODE_LEFT;
  } else if (rightBtnPressed)
  {
    leftBtnPressed = false;
    rightBtnPressed = false;
    mode = HANDLER_TYPE::MODE_RIGHT;
  }
  
  handler->drawAlarmDisableTimer(mode);
  handler->updateScreen();
  delay(1000);
  handler->changeSeconds(2, mode);
  handler->updateScreen();
  delay(1000);
  handler->changeSeconds(1, mode);
  handler->updateScreen();*/
}