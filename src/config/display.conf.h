#ifndef DISPLAY_CONF_H
#define DISPLAY_CONF_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ui/DisplayHandler.cpp>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define DISPLAY_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define DISPLAY_TYPE Adafruit_SSD1306
#define HANDLER_TYPE DisplayHandler<DISPLAY_TYPE>

#define UI_DISPLAY DISPLAY_TYPE(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, DISPLAY_RESET);
#define INIT_DISPLAY(_d) _d.begin(SSD1306_SWITCHCAPVCC, 0x3C);

#endif