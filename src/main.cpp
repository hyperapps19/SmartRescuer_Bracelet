#include <config/display.conf.h>
#include <ui/statuses.h>
#include <mqtt/MqttHandler.cpp>

DISPLAY_TYPE display; // !!! IMPORTANT !!! Must be GLOBAL or Exception will happen

void setup()
{
  Serial.begin(9600);
  display = UI_DISPLAY;
  INIT_DISPLAY(display);
  HANDLER_TYPE handler = HANDLER_TYPE(display);
  handler.drawLoadingScreen();
  handler.drawLoadStatus(STATUS_WIFI_CONN);
  handler.updateScreen();
  //MqttHandler *h = new MqttHandler(&handler);

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
