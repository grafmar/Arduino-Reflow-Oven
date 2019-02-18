#include "Display.h"
#include "ReflowOven.h"

#define xCountDivisionMark 11
#define xCountDivisionText 6
#define yCountDivisionMark 7
#define yCountDivisionText 6

#define xOffsetChart 25
#define yOffsetChart 186

// define the tft-object with the right library
#ifdef USE_ST7781
SWTFT tft;
#elif defined(USE_SPFD5408)
TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#endif // USE_ST7781

Display::Display() :
    actualScreen(homeScreen)
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

void Display::drawHomeScreen(bool isStarted, bool isFinish, Setpoint SollTempPoints[]) {
    tft.fillScreen(WHITE);
    tft.drawLine(0, 200, tft.width() - 1, 200, BLACK);
    tft.drawLine(106, 201, 106, tft.height() - 1, BLACK);
    tft.drawLine(213, 201, 213, tft.height() - 1, BLACK);

    tft.setCursor(5, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("Settings"));

    tft.setCursor(128, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);

    Serial.println(isStarted);
    Serial.println(isFinish);

    if (isStarted == true && isFinish == false) {   //during reflow process
        tft.println(F("Stop"));
    }
    else if (isStarted == false && isFinish == true) {   //end of reflow process
        tft.println(F("Reset"));
    }
    else {
        tft.println(F("Start"));
    }

    tft.setCursor(238, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("About"));

    tft.drawLine(270, 15, 280, 15, BLUE);     // Label soll
    tft.drawLine(270, 16, 280, 16, BLUE);
    tft.setCursor(282, 13);
    tft.setTextColor(BLUE);  tft.setTextSize(1);
    tft.println(F("Soll"));

    tft.drawLine(270, 25, 280, 25, RED);     // Label ist
    tft.drawLine(270, 26, 280, 26, RED);
    tft.setCursor(282, 23);
    tft.setTextColor(RED);  tft.setTextSize(1);
    tft.println(F("Ist"));

    drawChartAxis(SollTempPoints);
    drawSollLine(BLUE, false, SollTempPoints);
    actualScreen = homeScreen;
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

void Display::drawIstTemp(uint16_t istTemp[]) {
    for (uint16_t i = 0; i < 301; i++) {
        if (istTemp[i] > 0) {
            tft.drawPixel(int16_t(i * 250.0 / 300 + 25), int16_t(186 - istTemp[i] * 156.0 / 300), RED);
        }
    }
}

void Display::drawActualTemp(float actTemp, bool heater) {
    if (heater == true) {
        tft.fillRect(130, 0, 75, 25, RED);
    }
    else {
        tft.fillRect(130, 0, 75, 25, WHITE);
    }

    tft.setCursor(130, 5);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(actTemp);
}

void Display::drawSettingsScreen(void) {
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
    tft.println(F("Version V1.1"));

    tft.setCursor(10, 125);
    tft.println(F("Roman Scheuss &"));
    tft.setCursor(10, 150);
    tft.println(F("Marco Graf"));

    actualScreen = settingsScreen;
}

void Display::drawTempPointValueScreen(int16_t value) {
    tft.fillRect(81, 0, tft.width() - 1, 90, WHITE);
    tft.setCursor(100, 35);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(value);
}

void Display::drawTempInputScreen(bool isTempSelected, uint8_t selectedTempPoint, uint16_t &selectedTempPointValue, Setpoint SollTempPoints[]) {
    int counter = 1;
    tft.fillScreen(WHITE);

    // draw grid
    for (uint16_t i = 90; i < tft.height() - 1; i += 50) {
        tft.drawLine(0, i, tft.width() - 1, i, BLACK);
    }
    for (uint16_t i = 160; i < tft.width() - 1; i += 80) {
        tft.drawLine(i, 90, i, tft.height() - 1, BLACK);
    }
    tft.drawLine(80, 0, 80, tft.height() - 1, BLACK);
    tft.drawLine(0, 45, 80, 45, BLACK);


    // draw numbers in grid
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    for (uint16_t j = 104; j < tft.height() - 1; j += 50) {
        for (uint16_t i = 33; i < 240; i += 80) {
            tft.setCursor(i, j);
            tft.println(counter);
            counter++;
        }
    }

    tft.setCursor(257, 104);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(F("Del"));

    tft.setCursor(273, 154);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(F("0"));

    tft.setCursor(264, 204);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(F("OK"));

    if (isTempSelected) {
        tft.fillRect(0, 0, 80, 45, GREEN);
        selectedTempPointValue = SollTempPoints[selectedTempPoint].T;
    }
    else {
        tft.fillRect(0, 46, 80, 44, GREEN);
        selectedTempPointValue = SollTempPoints[selectedTempPoint].t;
    }

    tft.setCursor(5, 15);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(F("Temp"));

    tft.setCursor(5, 60);
    tft.setTextColor(BLACK);  tft.setTextSize(3);
    tft.println(F("Time"));

    drawTempPointValueScreen(selectedTempPointValue);

    actualScreen = tempInputScreen;
}

void Display::drawSollTempScreen(Setpoint SollTempPoints[]) {
    tft.fillScreen(WHITE);
    tft.drawLine(0, 200, tft.width() - 1, 200, BLACK);

    tft.drawLine(54, 201, 54, tft.height() - 1, BLACK);
    tft.drawLine(107, 201, 107, tft.height() - 1, BLACK);
    tft.drawLine(161, 201, 161, tft.height() - 1, BLACK);
    tft.drawLine(214, 201, 214, tft.height() - 1, BLACK);
    tft.drawLine(268, 201, 268, tft.height() - 1, BLACK);


    tft.setCursor(70, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("P1"));

    tft.setCursor(123, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("P2"));

    tft.setCursor(177, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("P3"));

    tft.setCursor(230, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("P4"));

    tft.setCursor(284, 213);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println(F("P5"));

    drawChartAxis(SollTempPoints);
    drawSollLine(BLUE, true, SollTempPoints);
    drawArrow(12, 220);
    actualScreen = sollTempScreen;
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

void Display::drawSollLine(uint16_t color, boolean drawIndicators, Setpoint SollTempPoints[]) {
    int16_t x1, x2, y1, y2;

    x1 = int16_t(SollTempPoints[0].t * 25 / 30.0 + xOffsetChart);
    y1 = int16_t(yOffsetChart - SollTempPoints[0].T * 26 / 50.0);
    x2 = int16_t(SollTempPoints[1].t * 25 / 30.0 + xOffsetChart);
    y2 = int16_t(yOffsetChart - SollTempPoints[1].T * 26 / 50.0);
    tft.drawLine(x1, y1, x2, y2, color);

    x1 = int16_t(SollTempPoints[1].t * 25 / 30.0 + xOffsetChart);
    y1 = int16_t(yOffsetChart - SollTempPoints[1].T * 26 / 50.0);
    x2 = int16_t(SollTempPoints[2].t * 25 / 30.0 + xOffsetChart);
    y2 = int16_t(yOffsetChart - SollTempPoints[2].T * 26 / 50.0);
    tft.drawLine(x1, y1, x2, y2, color);

    if (drawIndicators) {
        tft.setCursor(x1 - 10, y1 - 20);
        tft.setTextColor(BLACK);  tft.setTextSize(2);
        tft.println(F("P1"));
    }

    x1 = int16_t(SollTempPoints[2].t * 25 / 30.0 + xOffsetChart);
    y1 = int16_t(yOffsetChart - SollTempPoints[2].T * 26 / 50.0);
    x2 = int16_t(SollTempPoints[3].t * 25 / 30.0 + xOffsetChart);
    y2 = int16_t(yOffsetChart - SollTempPoints[3].T * 26 / 50.0);
    tft.drawLine(x1, y1, x2, y2, color);

    if (drawIndicators) {
        tft.setCursor(x1 - 10, y1 - 20);
        tft.setTextColor(BLACK);  tft.setTextSize(2);
        tft.println(F("P2"));
    }

    x1 = int16_t(SollTempPoints[3].t * 25 / 30.0 + xOffsetChart);
    y1 = int16_t(yOffsetChart - SollTempPoints[3].T * 26 / 50.0);
    x2 = int16_t(SollTempPoints[4].t * 25 / 30.0 + xOffsetChart);
    y2 = int16_t(yOffsetChart - SollTempPoints[4].T * 26 / 50.0);
    tft.drawLine(x1, y1, x2, y2, color);

    if (drawIndicators) {
        tft.setCursor(x1 - 10, y1 - 20);
        tft.setTextColor(BLACK);  tft.setTextSize(2);
        tft.println(F("P3"));
    }

    x1 = int16_t(SollTempPoints[4].t * 25 / 30.0 + xOffsetChart);
    y1 = int16_t(yOffsetChart - SollTempPoints[4].T * 26 / 50.0);
    x2 = int16_t(SollTempPoints[5].t * 25 / 30.0 + xOffsetChart);
    y2 = int16_t(yOffsetChart - SollTempPoints[5].T * 26 / 50.0);
    tft.drawLine(x1, y1, x2, y2, color);

    if (drawIndicators) {
        tft.setCursor(x1 - 10, y1 - 20);
        tft.setTextColor(BLACK);  tft.setTextSize(2);
        tft.println(F("P4"));
        tft.setCursor(x2 - 10, y2 - 20);
        tft.setTextColor(BLACK);  tft.setTextSize(2);
        tft.println(F("P5"));
    }
}

void Display::drawChartAxis(Setpoint SollTempPoints[]) {
    tft.drawLine(25, 20, 25, 188, BLACK);       // vertical axis
    tft.drawLine(25, 20, 23, 22, BLACK);       // Y arrow
    tft.drawLine(25, 20, 27, 22, BLACK);       // Y arrow
    tft.setCursor(18, 10);
    tft.setTextColor(BLACK);  tft.setTextSize(1);
    tft.println(F("T[C]"));

    tft.drawLine(23, 186, 300, 186, BLACK);     // horizontal axis
    tft.drawLine(298, 184, 300, 186, BLACK);     // X arrow
    tft.drawLine(298, 188, 300, 186, BLACK);     // X arrow
    tft.setCursor(290, 175);
    tft.setTextColor(BLACK);  tft.setTextSize(1);
    tft.println(F("t[s]"));

    // divisionsmark and text X-axis
    int xDivisionMark[xCountDivisionMark] ={ 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275 };
    const char* xDivisionText[xCountDivisionText] ={ " 0 ", " 60", "120", "180", "240", "300" };

    for (uint16_t i = 0; i < xCountDivisionMark; i++) {
        tft.drawLine(xDivisionMark[i], 186, xDivisionMark[i], 188, BLACK);      // draw division mark X-axis
    }

    for (uint16_t i = 0; i < xCountDivisionText; i++) {
        tft.setCursor(xDivisionMark[2 * i] - 8, 191);
        tft.setTextColor(BLACK);  tft.setTextSize(1);
        tft.println(xDivisionText[i]);
    }


    // divisionsmark and text Y-axis
    int yDivisionMark[yCountDivisionMark] ={ 30, 56, 82, 108, 134, 160, 186 };
    const char* yDivisionText[yCountDivisionText] ={ "300", "250", "200", "150", "100", " 50" };

    for (uint16_t i = 0; i < yCountDivisionMark; i++) {
        tft.drawLine(23, yDivisionMark[i], 25, yDivisionMark[i], BLACK);      // draw division mark X-axis
    }

    for (uint16_t i = 0; i < yCountDivisionText; i++) {
        tft.setCursor(3, yDivisionMark[i] - 3);
        tft.setTextColor(BLACK);  tft.setTextSize(1);
        tft.println(yDivisionText[i]);
    }
}

void Display::drawArrow(int16_t x, int16_t y) {
    tft.fillTriangle(x, y, x + 15, y + 10, x + 15, y - 10, BLACK);
    tft.fillRect(x + 15, y - 5, 15, 11, BLACK);
}

