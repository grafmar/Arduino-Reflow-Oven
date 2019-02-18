#include "TouchButton.h"
#include "ReflowOven.h"


TouchButton::TouchButton() :
    // For better pressure precision, we need to know the resistance
    // between X+ and X- Use any multimeter to read it
    // For the one we're using, its 300 ohms across the X plate
    ts(XP, YP, XM, YM, 300)
{
}


TouchButton::~TouchButton() {
}

TouchButton::buttons TouchButton::getTouchedButton(Display::screen currentScreen) {
    // get touched point
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    // map to display size
    uint16_t x = map(p.y, TS_MINY, TS_MAXY, 0, 320);
    uint16_t y = map(p.x, TS_MINX, TS_MAXX, 240, 0);

    if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
        return noButton;
    }
    Serial.print(F("Touch: "));
    Serial.print(x, DEC);
    Serial.print(F(", "));
    Serial.println(y, DEC);

    switch (currentScreen) {
        case Display::homeScreen:
            if (x < 106 && x > 0 && y < 240 && y > 200) {
                return buttonSollTemp;
            }
            if (x < 213 && x > 106 && y < 240 && y > 200) {
                return buttonStartStopReset;
            }
            if (x < 320 && x > 213 && y < 240 && y > 200) {
                return buttonSettings;
            }
            else {
                return noButton;
            }

        case Display::sollTempScreen:
            if (x < 54 && x > 0 && y < 240 && y > 200) {
                return buttonBack;
            }
            if (x < 107 && x > 54 && y < 240 && y > 200) {
                return buttonP1;
            }
            if (x < 161 && x > 107 && y < 240 && y > 200) {
                return buttonP2;
            }
            if (x < 214 && x > 161 && y < 240 && y > 200) {
                return buttonP3;
            }
            if (x < 268 && x > 214 && y < 240 && y > 200) {
                return buttonP4;
            }
            if (x < 320 && x > 268 && y < 240 && y > 200) {
                return buttonP5;
            }
            else {
                return noButton;
            }

        case Display::tempInputScreen:
            if (x < 80 && x > 0 && y < 140 && y > 90) {
                return button1;
            }
            if (x < 160 && x > 80 && y < 140 && y > 90) {
                return button2;
            }
            if (x < 240 && x > 160 && y < 140 && y > 90) {
                return button3;
            }

            if (x < 80 && x > 0 && y < 190 && y > 140) {
                return button4;
            }
            if (x < 160 && x > 80 && y < 190 && y > 140) {
                return button5;
            }
            if (x < 240 && x > 160 && y < 190 && y > 140) {
                return button6;
            }

            if (x < 80 && x > 0 && y < 240 && y > 190) {
                return button7;
            }
            if (x < 160 && x > 80 && y < 240 && y > 190) {
                return button8;
            }
            if (x < 240 && x > 160 && y < 240 && y > 190) {
                return button9;
            }

            if (x < 320 && x > 240 && y < 140 && y > 90) {
                return buttonDel;
            }
            if (x < 320 && x > 240 && y < 190 && y > 140) {
                return button0;
            }
            if (x < 320 && x > 240 && y < 240 && y > 190) {
                return buttonOK;
            }
            if (x < 80 && x > 0 && y < 45 && y > 0) {
                return buttonTemp;
            }
            if (x < 80 && x > 0 && y < 90 && y > 45) {
                return buttonTime;
            }

            else {
                return noButton;
            }

        case Display::settingsScreen:
            if (x < 54 && x > 0 && y < 240 && y > 200) {
                return buttonBack;
            }

            else {
                return noButton;
            }

        default:
            return noButton;
    }
}

