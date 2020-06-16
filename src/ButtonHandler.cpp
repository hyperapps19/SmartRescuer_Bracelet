#include <Arduino.h>
#include <functional>
#include <FunctionalInterrupt.h>

using namespace std;
class ButtonHandler
{
    // Меньше интервал - можно быстрее нажимать на кнопку, но больше риск ложного срабатывания
    static const uint16_t DEBOUNCE_INTERVAL_MS = 50;

public:
    ButtonHandler(uint8_t pin, uint8_t mode = INPUT_PULLUP)
    {
        this->interruptPin = pin;
        pinMode(pin, mode);
        attachInterrupt(pin, std::bind(&ButtonHandler::onChange, this), CHANGE);
    }

    void setCallback(function<void()> f)
    {
        callbackF = f;
    }
    ~ButtonHandler()
    {
        detachInterrupt(interruptPin);
    }

private:
    unsigned long previousTime = 0;
    uint8_t interruptPin;
    bool deTwicer = true;
    void onChange(void)
    {
        if ((millis() - previousTime) > DEBOUNCE_INTERVAL_MS)
        {
            if (deTwicer)
                callbackF();
            deTwicer = !deTwicer;
        }
        previousTime = millis();
    }
    function<void()> callbackF;
};