#include <DisplayHandler.cpp>
/*

  Hello_Adafruit_SSD1306.ino

  Demonstrates how to connect U8g2_for_Adafruit_GFX to Adafruit SSD1306 library.

  U8g2_for_Adafruit_GFX:
    - Use U8g2 fonts with Adafruit GFX
    - Support for UTF-8 in print statement
    - 90, 180 and 270 degree text direction
  
  List of all U8g2 fonts: https://github.com/olikraus/u8g2/wiki/fntlistall
      
*/
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DISPLAY_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, DISPLAY_RESET);

void setup()
{
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  DisplayHandler<Adafruit_SSD1306> handler(display); // connect u8g2 procedures to Adafruit GFX
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
