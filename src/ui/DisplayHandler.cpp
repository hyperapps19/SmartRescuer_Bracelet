#ifndef DISPLAY_HANDLER
#define DISPLAY_HANDLER
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <ui/images.h>

template <typename DisplayType>
class DisplayHandler
{
public:
    DisplayHandler(DisplayType &d)
    {
        this->d = d;
        gfx8.begin(d);
        gfx8.setForegroundColor(WHITE);
        d.clearDisplay();
    }

    void drawMainScreen(void)
    {
        gfx8.setFont(u8g2_font_courB24_tf);
        d.drawBitmap(5, 0, heart, 30, 30, WHITE);
        gfx8.setCursor(7, 55);
        gfx8.print('O');
        gfx8.setFont(u8g2_font_courB12_tf);
        gfx8.setCursor(26, 60);
        gfx8.print('2');
        d.drawFastVLine(77, 0, 64, WHITE);
        setPulseAndSpO2Values(-1, -1);
        setClock(0, 0);
    }

    void setClock(uint8_t hh, uint8_t mm)
    {
        String hhStr = String(hh);
        String mmStr = String(mm);
        if (hhStr.length() < 2)
            hhStr = "0" + hhStr;
        if (mmStr.length() < 2)
            mmStr = "0" + mmStr;

        d.fillRect(78, 0, 50, 64, BLACK);
        gfx8.setFont(u8g2_font_courB24_tf);
        gfx8.setCursor(85, 28);
        gfx8.print(hhStr);
        gfx8.setCursor(85, 57);
        gfx8.print(mmStr);
    }

#define EMPTY_VALUE "---"
    void setPulseAndSpO2Values(int32_t pulse, int32_t spo2)
    {
        d.fillRect(41, 0, 36, 64, BLACK);
        String pulStr, spo2Str;
        if (pulse < 0)
            pulStr = EMPTY_VALUE;
        else
            pulStr = String(pulse);

        if (spo2 < 0)
            spo2Str = EMPTY_VALUE;
        else
            spo2Str = String(spo2);

        if (spo2Str.length() < 3)
            spo2Str += "%";

        gfx8.setFont(u8g2_font_10x20_t_cyrillic);
        gfx8.setCursor(42, 20);
        gfx8.print(pulStr);
        gfx8.setCursor(42, 53);
        gfx8.print(spo2Str);
    }

    void drawAlarmScreen(void)
    {
        d.drawBitmap(0, 0, alarmImg, 51, 40, WHITE);
        d.drawBitmap(88, 0, logo, 40, 40, WHITE);
        d.drawBitmap(51, 10, arrowRight, 37, 22, WHITE);
        gfx8.setFont(u8g2_font_10x20_t_cyrillic);
        d.drawFastHLine(0, d.height() - 22, d.width(), WHITE);
        gfx8.setCursor(5, d.height() - 5);
        gfx8.print("Откл.тревогу");
    }

    static const bool MODE_LEFT = false;
    static const bool MODE_RIGHT = true;

    void drawAlarmDisableTimer(bool mode)
    {
        gfx8.setFont(u8g2_font_10x20_t_cyrillic);
        d.fillRect(0, 43, 128, 21, BLACK);
        if (mode) // MODE_RIGHT
        {
            gfx8.setCursor(0, d.height() - 5);
            gfx8.print("3 сек.");
            d.drawFastHLine(64, d.height() - 21, 64, WHITE);
            d.drawFastHLine(64, d.height() - 20, 64, WHITE);

            d.drawFastVLine(64, d.height() - 22, 22, WHITE);
            d.drawFastVLine(65, d.height() - 22, 22, WHITE);
            d.drawFastVLine(66, d.height() - 22, 22, WHITE);
            gfx8.setCursor(70, d.height() - 5);
            gfx8.print("Удерж.");
        }
        else
        { // MODE_LEFT
            gfx8.setCursor(70, d.height() - 5);
            gfx8.print("3 сек.");
            d.drawFastHLine(0, d.height() - 21, 64, WHITE);
            d.drawFastHLine(0, d.height() - 20, 64, WHITE);

            d.drawFastVLine(64, d.height() - 22, 22, WHITE);
            d.drawFastVLine(63, d.height() - 22, 22, WHITE);
            d.drawFastVLine(62, d.height() - 22, 22, WHITE);
            gfx8.setCursor(0, d.height() - 5);
            gfx8.print("Удерж.");
        }
    }

    bool changeSeconds(uint8_t newSeconds, bool mode)
    {
        if (mode) // MODE_RIGHT
        {
            d.fillRect(0, 45, 20, 20, BLACK);
            gfx8.setCursor(0, d.height() - 5);
        }
        else
        { // MODE_LEFT
            d.fillRect(70, 45, 20, 20, BLACK);
            gfx8.setCursor(70, d.height() - 5);
        }
        gfx8.print(String(newSeconds));
    }

    void drawYesNoButtons(void)
    {
        gfx8.setFont(u8g2_font_10x20_t_cyrillic);
        d.drawFastHLine(0, d.height() - 25, d.width(), WHITE);
        gfx8.setCursor(20, d.height() - 5);
        gfx8.print("ДА");
        gfx8.setCursor(d.width() - 45, d.height() - 5);
        gfx8.print("НЕТ");
        d.drawFastVLine(d.width() / 2, d.height() - 25, 25, WHITE);
    }

    void drawIsHelpHeeded(void)
    {
        gfx8.setCursor(48, 15);
        gfx8.print("Нужна");
        gfx8.setCursor(48, gfx8.getCursorY() + 3);
        gfx8.print("помощь?");
        gfx8.setCursor(10, 30);
        gfx8.setFont(u8g2_font_inr24_t_cyrillic);
        d.fillCircle(19, 17, 17, WHITE);
        d.fillCircle(19, 17, 15, BLACK);
        gfx8.print("?");
    }

    void drawLoadingScreen(void)
    {
        d.drawBitmap(0, 0, logo, 40, 40, WHITE);
        gfx8.setCursor(60, 18);
        gfx8.setFont(u8g2_font_10x20_t_cyrillic);
        gfx8.print("УМНЫЙ");
        gfx8.setCursor(40, 37);
        gfx8.print("СПАСАТЕЛЬ");
        d.drawFastHLine(40, 39, d.width() - 40, WHITE);
    }

    void drawLoadStatus(const char *status)
    {
        gfx8.setFont(u8g2_font_9x15_t_cyrillic);
        d.fillRect(0, 40, d.width(), d.height() - 40, BLACK);
        gfx8.setCursor(getCenteredTextStartPosition(status), 60);
        gfx8.print(status);
    }
    void updateScreen() { d.display(); }
    void clearScreen() { d.clearDisplay(); }
    friend class StartScreenStatusDrawer;

private:
    Adafruit_SSD1306 d;
    U8G2_FOR_ADAFRUIT_GFX gfx8;
    inline uint16_t getCenteredTextStartPosition(const char *str)
    {
        return (d.width() - gfx8.getUTF8Width(str)) / 2;
    }

    //void *operator new(size_t size); // overloaded and private
};
#endif