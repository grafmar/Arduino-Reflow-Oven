#include "Display.h"
#include "EepromHandler.h"
#include "ReflowOven.h"
#include "TouchButton.h"

#define xCountDivisionMark 11
#define xCountDivisionText 6
#define yCountDivisionMark 7
#define yCountDivisionText 6

const char* const buttonText[] PROGMEM = {
    ("Settings"),   // buttonSettings,
    ("About"),      // buttonAboutInfo,
    (""),           // buttonBack,
    ("Edit"),       // buttonEdit,
    ("Load"),       // buttonLoad,
    ("Save"),       // buttonSave,
    (""),           // buttonBackSettings,
    ("P1"),         // buttonP1,
    ("P2"),         // buttonP2,
    ("P3"),         // buttonP3,
    ("P4"),         // buttonP4,
    ("P5"),         // buttonP5,
    ("0"),          // button0,
    ("1"),          // button1,
    ("2"),          // button2,
    ("3"),          // button3,
    ("4"),          // button4,
    ("5"),          // button5,
    ("6"),          // button6,
    ("7"),          // button7,
    ("8"),          // button8,
    ("9"),          // button9,
    ("Del"),        // buttonDel,
    ("OK"),         // buttonOK,
    (""),           // buttonStartStopReset,
    ("Time"),       // buttonTime,
    ("Temp"),       // buttonTemp,
    ("Default (Startup)"),    // buttonDefault,
    (""),           // buttonM1,
    (""),           // buttonM2,
    (""),           // buttonM3,
    (""),           // buttonM4,
    (""),           // buttonM5,
    ("Save"),       // buttonSaveAs,
    ("")            // noButton
};

// define the tft-object with the right library
#ifdef USE_ST7781
SWTFT tft;
#elif defined(USE_SPFD5408)
TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif // USE_ST7781

Display::Display(Setpoints& setpoints, ProcessState& processState) :
    m_setpoints(setpoints),
    m_processState(processState),
    actualScreen(homeScreen),
    lastChartTemperature(0U),
    maxTimeInChart(300U)
{
}

Display::~Display() {
}

void Display::begin() {
    tft.reset();
    uint16_t identifier = tft.readID();
    Serial.print(F("Identifier: "));
    Serial.println(identifier, HEX);
    identifier = 0x9341;
    tft.begin(identifier);
    tft.setRotation(ROTATION);
}

void Display::drawHomeScreen() {
    actualScreen = homeScreen;

    if (m_processState == ProcessState::Finished) {
        tft.fillRect(0, 200, tft.width(), tft.height() - 200, WHITE);
    } else {
        tft.fillScreen(WHITE);
    }
    drawButtons(actualScreen);

    tft.drawRect(270, 16, 10, 2, BLUE);     // Label soll
    tft.setCursor(282, 13);
    tft.setTextColor(BLUE);  tft.setTextSize(1);
    tft.println(F("Soll"));

    tft.drawRect(270, 26, 10, 2, RED);     // Label ist
    tft.setCursor(282, 23);
    tft.setTextColor(RED);  tft.setTextSize(1);
    tft.println(F("Ist"));

    maxTimeInChart = m_setpoints.sp[5].time;
    drawChartAxis();
    drawSollLine(BLUE, false);
}

void Display::drawRemainingTime(uint16_t remainingTime) {
    tft.fillRect(100, 160, 130, 25, WHITE);
    if (remainingTime > 0) {
        tft.setCursor(100, 165);
        tft.setTextColor(BLACK);  tft.setTextSize(2);
        tft.print(F("preHeat: "));
        tft.print(remainingTime);
    }
}

void Display::drawActualTemperatueChart(uint16_t time, uint16_t temperature) {
    uint16_t lastTime = time - 1;
    if (time == 0) {
        lastChartTemperature = temperature;
        lastTime = 0;
    }
    const int16_t x1Coord = static_cast<int16_t>(map(lastTime, 0, maxTimeInChart, CHART_AREA_X1, CHART_AREA_X2));
    const int16_t x2Coord = static_cast<int16_t>(map(time, 0, maxTimeInChart, CHART_AREA_X1, CHART_AREA_X2));
    const int16_t y1Coord = static_cast<int16_t>(map(lastChartTemperature, 0, MAX_TEMPERATURE, CHART_AREA_Y1, CHART_AREA_Y2));
    const int16_t y2Coord = static_cast<int16_t>(map(temperature, 0, MAX_TEMPERATURE, CHART_AREA_Y1, CHART_AREA_Y2));
    tft.drawLine(x1Coord, y1Coord, x2Coord, y2Coord, RED);
    lastChartTemperature = temperature;
}

void Display::drawActualTemp(float actTemp, bool heater) {
    tft.fillRect(130, 0, 75, 25, ((heater == true) ? RED : WHITE));
    tft.setCursor(130, 5);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(actTemp);
}

void Display::drawAboutInfoScreen(void) {
    actualScreen = aboutInfoScreen;

    tft.fillScreen(WHITE);

    tft.drawLine(0, 200, 54, 200, BLACK);
    tft.drawLine(54, 201, 54, tft.height() - 1, BLACK);

    drawArrow(12, 220);

    tft.setTextColor(BLACK);

    tft.setTextSize(3);
    tft.setCursor(10, 25);
    tft.println(F("Reflow Oven"));

    tft.setTextSize(2);
    tft.setCursor(10, 75);
    tft.print(F("Version V"));
    tft.print(VERSION_MAJOR);
    tft.print(F("."));
    tft.println(VERSION_MINOR);


    tft.setCursor(10, 125);
    tft.println(F("Roman Scheuss &"));
    tft.setCursor(10, 150);
    tft.println(F("Marco Graf"));
}

void Display::drawTempPointValueScreen(uint16_t value) {
    tft.fillRect(81, 0, tft.width() - 1, 90, WHITE);
    tft.setCursor(100, 35);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(value);
}

void Display::drawSettingsScreen(void) {
    actualScreen = settingsScreen;

    tft.fillScreen(WHITE);
    drawButtons(actualScreen);
    drawArrow(12, 220);
    maxTimeInChart = m_setpoints.sp[5].time;
    drawChartAxis();
    drawSollLine(BLUE, true);
}

void Display::drawEditSetpointsScreen() {
    actualScreen = editSetpointsScreen;

    tft.fillScreen(WHITE);
    drawButtons(actualScreen);
    drawArrow(12, 220);
    maxTimeInChart = m_setpoints.sp[5].time;
    drawChartAxis();
    drawSollLine(BLUE, true);
}

void Display::drawSetpointInputScreen(bool isTempSelected, uint8_t selectedTempPoint, uint16_t &selectedTempPointValue) {
    actualScreen = setpointInputScreen;

    int counter = 1;
    tft.fillScreen(WHITE);

    if (isTempSelected) {
        tft.fillRect(0, 0, 80, 45, GREEN);
        selectedTempPointValue = m_setpoints.sp[selectedTempPoint].temperature;
    }
    else {
        tft.fillRect(0, 46, 80, 44, GREEN);
        selectedTempPointValue = m_setpoints.sp[selectedTempPoint].time;
    }

    drawButtons(actualScreen);

    drawTempPointValueScreen(selectedTempPointValue);
}

void Display::drawLoadSetpointsScreen() {
    actualScreen = loadSetpointsScreen;

    tft.fillScreen(WHITE);
    drawButtons(actualScreen);
    drawArrow(12, 220);

    tft.setTextColor(BLACK);  tft.setTextSize(2);
    for (uint8_t i=0; i < 5; i++) {
        tft.setCursor(5, 40*i + 14);
        String buttonText = String("M") + String(i+1) + ": " + EepromHandler::getSetpointSetName(i+1);
        tft.print(buttonText);
    }
}

void Display::drawSaveSetpointsScreen() {
    drawLoadSetpointsScreen();
    actualScreen = saveSetpointsScreen;
}

void Display::drawEnterNameScreen(String name) {
    actualScreen = enterNameScreen;

    tft.fillScreen(WHITE);
    drawButtons(actualScreen);
    drawArrow(12, 220);

    tft.setCursor(5, 5);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("Name:"));

    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.setCursor(10, 40);
    tft.println(F("A  B  C  D  E  F  G  H  I"));
    tft.setCursor(10, 70);
    tft.println(F("J  K  L  M  N  O  P  Q  R"));
    tft.setCursor(10, 100);
    tft.println(F("S  T  U  V  W  X  Y  Z  0"));
    tft.setCursor(10, 130);
    tft.println(F("1  2  3  4  5  6  7  8  9"));
    tft.setCursor(10, 160);
    tft.println(F(",  .  _   Space   Del"));

    updateNameValue(name);
}

void Display::updateNameValue(String name) {
    tft.fillRect(65, 0, 320-65, 25, WHITE);
    tft.setCursor(65, 5);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(name);
    const int16_t xpos = 65 + name.length() * 2 * 6;
    tft.drawLine(xpos, 3, xpos, 23, BLACK);
}

void Display::drawErrorScreen(String message) {
    tft.fillScreen(RED);
    tft.setCursor(20, 100);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(message);
}

Display::screen Display::getActualScreen() {
    return actualScreen;
}

void Display::drawSollLine(uint16_t color, boolean drawIndicators) {
    static const uint8_t NUM_OF_SETPOINTS = 6;
    int16_t x1, x2, y1, y2;

    for (uint8_t i=0; i<(NUM_OF_SETPOINTS - 1); i++) {
        const int16_t x1Coord = static_cast<int16_t>(map(m_setpoints.sp[i].time, 0, maxTimeInChart, CHART_AREA_X1, CHART_AREA_X2));
        const int16_t x2Coord = static_cast<int16_t>(map(m_setpoints.sp[i + 1].time, 0, maxTimeInChart, CHART_AREA_X1, CHART_AREA_X2));
        const int16_t y1Coord = static_cast<int16_t>(map(m_setpoints.sp[i].temperature, 0, MAX_TEMPERATURE, CHART_AREA_Y1, CHART_AREA_Y2));
        const int16_t y2Coord = static_cast<int16_t>(map(m_setpoints.sp[i + 1].temperature, 0, MAX_TEMPERATURE, CHART_AREA_Y1, CHART_AREA_Y2));
        tft.drawLine(x1Coord, y1Coord, x2Coord, y2Coord, color);

        if (drawIndicators) {
            tft.setCursor(x2Coord - 5, y2Coord - 10);
            tft.setTextColor(BLACK);  tft.setTextSize(1);
            tft.print(F("P"));
            tft.println(i+1);
        }
    }
}

void Display::drawChartAxis() {
    tft.drawLine(CHART_AREA_X1, 20, CHART_AREA_X1, CHART_AREA_Y1, BLACK);       // vertical axis
    tft.drawLine(CHART_AREA_X1, 20, CHART_AREA_X1-2, 22, BLACK);       // Y arrow
    tft.drawLine(CHART_AREA_X1, 20, CHART_AREA_X1+2, 22, BLACK);       // Y arrow
    tft.setCursor(18, 10);
    tft.setTextColor(BLACK);  tft.setTextSize(1);
    tft.println(F("T[C]"));

    tft.drawLine(CHART_AREA_X1, CHART_AREA_Y1, 300, CHART_AREA_Y1, BLACK);     // horizontal axis
    tft.drawLine(298, CHART_AREA_Y1-2, 300, CHART_AREA_Y1, BLACK);     // X arrow
    tft.drawLine(298, CHART_AREA_Y1+2, 300, CHART_AREA_Y1, BLACK);     // X arrow
    tft.setCursor(290, 175);
    tft.setTextColor(BLACK);  tft.setTextSize(1);
    tft.println(F("t[s]"));

    // divisionsmark and text X-axis
    //const uint16_t timeStep = 60U;
    uint16_t minTimeStep = 1;
    if (maxTimeInChart >= 3 * 1800U) {
        minTimeStep = 1800U;
    } else if (maxTimeInChart >= 3 * 600U) {
        minTimeStep = 600U;
    } else if (maxTimeInChart >= 3 * 60U) {
        minTimeStep = 60U;
    } else if (maxTimeInChart >= 3 * 10U) {
        minTimeStep = 10U;
    }
    const uint16_t timeStep = (maxTimeInChart + (5U * minTimeStep - 1U)) / (5U * minTimeStep) * minTimeStep;
    for (uint16_t time = 0; time <= maxTimeInChart; time+=timeStep) {
        const int16_t xCoord = static_cast<int16_t>(map(time, 0, maxTimeInChart, CHART_AREA_X1, CHART_AREA_X2));
        tft.drawLine(xCoord, CHART_AREA_Y1, xCoord, CHART_AREA_Y1 + 2, BLACK);      // draw division mark X-axis
        char textBuffer[7];
        sprintf(textBuffer, "%d", time);
        tft.setCursor(xCoord - (strlen(textBuffer)*6)/2 + 1, 191);
        tft.setTextColor(BLACK);  tft.setTextSize(1);
        tft.println(textBuffer);

        // small marks
        if ((time + timeStep/2) < maxTimeInChart) {
            const int16_t xCoord = static_cast<int16_t>(map(time + timeStep / 2, 0, maxTimeInChart, CHART_AREA_X1, CHART_AREA_X2));
            tft.drawLine(xCoord, CHART_AREA_Y1, xCoord, CHART_AREA_Y1 + 2, BLACK);      // draw division mark X-axis
        }
    }

    // divisionsmark and text Y-axis
    const uint16_t temperatureStep = 50U;
    for (uint16_t temperature = 0; temperature <= MAX_TEMPERATURE; temperature+=temperatureStep) {
        const int16_t yCoord =  static_cast<int16_t>(map(temperature, 0, MAX_TEMPERATURE, CHART_AREA_Y1, CHART_AREA_Y2));
        tft.drawLine(CHART_AREA_X1-2, yCoord, CHART_AREA_X1, yCoord, BLACK);      // draw division mark X-axis
        tft.setCursor(3, yCoord - 3);
        tft.setTextColor(BLACK);  tft.setTextSize(1);
        char textBuffer[5];
        sprintf(textBuffer, "%3d", temperature);
        tft.println(textBuffer);
    }
}

void Display::drawArrow(int16_t x, int16_t y) {
    tft.fillTriangle(x, y, x + 15, y + 10, x + 15, y - 10, BLACK);
    tft.fillRect(x + 15, y - 5, 15, 11, BLACK);
}

void Display::drawButtons(screen screenId) {
    tft.setTextColor(BLACK);
    tft.setTextSize(2);

    for (uint16_t i=0; i < TouchButton::NUM_OF_TOUCH_BUTTONS; i++) {
        TouchButton::TouchButtonElement actualTouchButton;
        memcpy_P(&actualTouchButton, &TouchButton::TOUCH_BUTTONS[i], sizeof(actualTouchButton));

        if (actualTouchButton.screen == screenId) {
            tft.drawRect(actualTouchButton.x1, actualTouchButton.y1, actualTouchButton.x2 - actualTouchButton.x1 + 1, actualTouchButton.y2 - actualTouchButton.y1 + 1, BLACK);
            String btnName = (char*)pgm_read_word(&(buttonText[actualTouchButton.buttonId]));
            // handle special button text
            if (actualTouchButton.buttonId == TouchButton::buttonStartStopReset) {
                if (m_processState == ProcessState::Ready) {
                    btnName = F("Start");
                }
                else if (m_processState == ProcessState::Running) {
                    btnName = F("Stop");
                }
                else {
                    btnName = F("Reset");
                }
            } else if ((actualTouchButton.buttonId == TouchButton::buttonSettings)
                || (actualTouchButton.buttonId == TouchButton::buttonAboutInfo)) {
                
                if (m_processState != ProcessState::Ready) {
                    btnName = F("");
                }
            }

            const int16_t xCoord = (actualTouchButton.x1 + actualTouchButton.x2) / 2 - btnName.length() * 2U * 6U / 2U + 1;
            const int16_t yCoord = (actualTouchButton.y1 + actualTouchButton.y2) / 2 - 7 + 1;
            tft.setCursor(xCoord, yCoord);
            tft.println(btnName);
        }
    }
}

