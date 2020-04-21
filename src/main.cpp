#include <config/display.conf.h>
#include <ui/statuses.h>

DISPLAY_TYPE display; // !!! IMPORTANT !!! Must be GLOBAL or Exception will happen

void setup()
{
  Serial.begin(9600);
  display = UI_DISPLAY;
  INIT_DISPLAY(display);
  HANDLER_TYPE handler = HANDLER_TYPE(display);
  handler.drawLoadingScreen();
  handler.drawLoadStatus("wifi падкл");
  handler.updateScreen();
  delay(1000);
  handler.drawLoadStatus("nqtt");
  handler.updateScreen();
  delay(2000);
  handler.drawLoadStatus("пулс");
  handler.updateScreen();
  delay(5000);
  handler.clearScreen();
  handler.drawYesNoButtons();
  handler.drawIsHelpHeeded();
  handler.updateScreen();
}

void loop()
{
}
