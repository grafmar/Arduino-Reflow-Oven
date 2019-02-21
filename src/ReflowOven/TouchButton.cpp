#include "TouchButton.h"
#include "ReflowOven.h"

const PROGMEM TouchButton::TouchButtonElement TouchButton::TOUCH_BUTTONS[TouchButton::NUM_OF_TOUCH_BUTTONS] = {
    { Display::homeScreen,        0, 106, 200, 240, TouchButton::buttonSollTemp },
    { Display::homeScreen,      106, 213, 200, 240, TouchButton::buttonStartStopReset },
    { Display::homeScreen,      213, 320, 200, 240, TouchButton::buttonSettings },
    
    { Display::sollTempScreen,    0,  54, 200, 240, TouchButton::buttonBack },
    { Display::sollTempScreen,   54, 107, 200, 240, TouchButton::buttonP1 },
    { Display::sollTempScreen,  107, 161, 200, 240, TouchButton::buttonP2 },
    { Display::sollTempScreen,  161, 214, 200, 240, TouchButton::buttonP3 },
    { Display::sollTempScreen,  214, 268, 200, 240, TouchButton::buttonP4 },
    { Display::sollTempScreen,  268, 320, 200, 240, TouchButton::buttonP5 },

    { Display::tempInputScreen,   0,  80,  90, 140, TouchButton::button1 },
    { Display::tempInputScreen,  80, 160,  90, 140, TouchButton::button2 },
    { Display::tempInputScreen, 160, 240,  90, 140, TouchButton::button3 },
    { Display::tempInputScreen,   0,  80, 140, 190, TouchButton::button4 },
    { Display::tempInputScreen,  80, 160, 140, 190, TouchButton::button5 },
    { Display::tempInputScreen, 160, 240, 140, 190, TouchButton::button6 },
    { Display::tempInputScreen,   0,  80, 190, 240, TouchButton::button7 },
    { Display::tempInputScreen,  80, 160, 190, 240, TouchButton::button8 },
    { Display::tempInputScreen, 160, 240, 190, 240, TouchButton::button9 },
    { Display::tempInputScreen, 240, 320,  90, 140, TouchButton::buttonDel },
    { Display::tempInputScreen, 240, 320, 140, 190, TouchButton::button0 },
    { Display::tempInputScreen, 240, 320, 190, 240, TouchButton::buttonOK },
    { Display::tempInputScreen,   0,  80,   0,  45, TouchButton::buttonTemp },
    { Display::tempInputScreen,   0,  80,  45,  90, TouchButton::buttonTime },
    
    { Display::settingsScreen,    0,  54, 200, 240, TouchButton::buttonBack}
};

TouchButton::TouchButton() :
    // For better pressure precision, we need to know the resistance
    // between X+ and X- Use any multimeter to read it
    // For the one we're using, its 300 ohms across the X plate
    ts(XP, YP, XM, YM, 300)
{
}

TouchButton::~TouchButton() {
}

TouchButton::ButtonId TouchButton::getTouchedButton(Display::screen currentScreen) {
    // get touched point
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    // map to display size
    uint16_t x = map(p.y, TS_MINY, TS_MAXY, 0, 320);
    uint16_t y = map(p.x, TS_MINX, TS_MAXX, 240, 0);

    // if touch event
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
        Serial.print(F("Touch: "));
        Serial.print(x, DEC);
        Serial.print(F(", "));
        Serial.println(y, DEC);

        for (uint16_t i=0; i < NUM_OF_TOUCH_BUTTONS; i++) {
            TouchButtonElement actualTouchButton;
            memcpy_P(&actualTouchButton, &TOUCH_BUTTONS[i], sizeof(actualTouchButton));
            if (actualTouchButton.screen == currentScreen) {
                if ((x > actualTouchButton.x1)
                    && (x < actualTouchButton.x2)
                    && (y > actualTouchButton.y1)
                    && (y < actualTouchButton.y2)) {
                
                    return actualTouchButton.buttonId;
                }
            }
        }
    }

    // No touch event, or no button not recognized
    return noButton;
}

