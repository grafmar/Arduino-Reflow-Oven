#include <Adafruit_GFX.h>    // Core graphics library
#include "SWTFT.h" // Hardware-specific library
#include <TouchScreen.h>
//#include <EEPROM.h>
#include <TimerOne.h>
#include <max6675.h>

// touchscreen definitons
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define MINPRESSURE 10
#define MAXPRESSURE 1000

// display definitons
// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define xCountDivisionMark 11
#define xCountDivisionText 6
#define yCountDivisionMark 7
#define yCountDivisionText 6

#define xOffsetChart 25
#define yOffsetChart 186

// thermocouple definitons
#define thermoDO 12
#define thermoCS 11
#define thermoCLK 10

// define heater pin
#define heaterPin 19

// generate objects
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
SWTFT tft; //touchscreen

// define enumerations (screens and buttons)
enum screens {homeScreen, sollTempScreen, tempInputScreen, settingsScreen, noChange};
screens actualScreen = homeScreen;

enum buttons {buttonSollTemp, buttonSettings, buttonBack, buttonP1, buttonP2, buttonP3, buttonP4, buttonP5, button0, button1, button2, button3, button4, button5, button6, button7, button8, button9,  buttonDel, buttonOK, buttonStartStopReset, buttonTime, buttonTemp, noButton};
buttons touchedButton = noButton;
buttons prevButton = noButton;
buttons actualInputSelection = buttonTemp;

int selectedTempPointValue = 0;
char selectedTempPoint = 0;

boolean isStarted = false;
boolean isFinish = false;
boolean timeToMeasure = false;
boolean heater = false;

int i, j;
uint16_t istTemp[301];
uint16_t sollTempLine[301];
uint16_t timeCounter = 0;
uint16_t preHeatTime = 12;

double temp;

struct TempPoint {
  int16_t t;
  int16_t T;
};
TempPoint SollTempPoints[6];

//--------------------------------------------------------------------------------------------------------------------------

void setup(void) {
  // initialize serial communication
  Serial.begin(115200);
  Serial.println(F("Startup... Ready"));

  // initialize the timer 1
  Timer1.initialize(1000000); // set the timer to 1sec
  Timer1.attachInterrupt(timerIsr); // attach the service routine here

  // define default temperature
  SollTempPoints[0].t = 0;
  SollTempPoints[0].T = 25;

  SollTempPoints[1].t = 30;
  SollTempPoints[1].T = 100;

  SollTempPoints[2].t = 120;
  SollTempPoints[2].T = 150;

  SollTempPoints[3].t = 150;
  SollTempPoints[3].T = 183;

  SollTempPoints[4].t = 210;
  SollTempPoints[4].T = 235;

  SollTempPoints[5].t = 240;
  SollTempPoints[5].T = 183;

  // clear istTemp array
  for (i = 0; i < 301; i++) {
    istTemp[i] = 0;
  }

  // reset and start display
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);

  // initialize heater pin
  pinMode(heaterPin, OUTPUT);

  // draw homescreen
  drawHomeScreen();
  delay(1000);

  //for(i = 0; i < 12; i++){
  // Serial.println(EEPROM.read(i));
  //}
  // for (i = 0; i < 6; i++) {
  // EEPROM.write(2 * i, SollTempPoints[i].t);
  // EEPROM.write(2 * i + 1, SollTempPoints[i].T);
  // }
}

//--------------------------------------------------------------------------------------------------------
void loop(void) {

  // get touched point
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  TSPoint point;
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  // map to display size
  point.x = map(p.y, TS_MINY, TS_MAXY, 0, 320);
  point.y = map(p.x, TS_MINX, TS_MAXX, 240, 0);
  point.z = p.z;

  if (timeToMeasure) {

    // read actual temperature
    temp = thermocouple.readCelsius();

    // safety deadlock--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if ((temp > 300) || isnan(temp)) {
      digitalWrite(heaterPin, LOW);
      tft.fillScreen(RED);
      tft.setCursor(20, 100);
      tft.setTextColor(BLACK);  tft.setTextSize(3);
      if (temp > 300) {
        tft.println(F("ERROR: TO HOT!!!"));
      }
      else {
        tft.println(F("FAIL SENSOR!!!"));
      }
      while (1);
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if (isStarted == true) {

      // to heat up during preheat time
      if (preHeatTime > 0) {
        heater = true;
        digitalWrite(heaterPin, HIGH);
        preHeatTime --;
        drawRemainingTime(preHeatTime);
      }

      // normal heat up
      else {
        istTemp[timeCounter] = uint16_t(temp);      // store istTemp in istTempArray

        // to turn on and off the heater if necessary
        if (istTemp[timeCounter] < sollTempLine[timeCounter]) {
          heater = true;
          digitalWrite(heaterPin, HIGH);
        }
        else {
          heater = false;
          digitalWrite(heaterPin, LOW);
        }

        // to terminate a reflow process
        if (timeCounter >= SollTempPoints[5].t) {
          timeCounter = 0;
          isStarted = false;
          isFinish = true;
          heater = false;
          digitalWrite(heaterPin, LOW);
          drawHomeScreen();
        }
        drawIstTemp(istTemp);
        timeCounter++;
      }
    }

    if ((actualScreen != tempInputScreen) && (actualScreen != settingsScreen)) {
      drawActualTemp(temp);
    }

    timeToMeasure = false;
  }

  // limit the pressure force
  if (point.z > MINPRESSURE && point.z < MAXPRESSURE) {

    touchedButton = getTouchedButton(actualScreen, point.x, point.y);

    //to block buttonSollTemp and buttonSettings during reflow process
    if (isStarted && (touchedButton == buttonSollTemp || touchedButton == buttonSettings)) {
      touchedButton = noButton;
      Serial.println("Locked -> noAction");
    }
    //to block buttonSollTemp and buttonSettings before press reset
    if (isFinish && (touchedButton == buttonSollTemp || touchedButton == buttonSettings)) {
      touchedButton = noButton;
      Serial.println("Locked -> noAction");
    }

    if (prevButton != touchedButton) {

      switch (actualScreen) {
        case homeScreen: {
            if (touchedButton == buttonSollTemp) {
              drawSollTempScreen();
            }

            if (touchedButton == buttonStartStopReset) {

              if (isStarted == false && isFinish == false) {  // starts reflow process
                isStarted = true;
                calcSollLine();
              }
              else {                // reset reflow process
                isStarted = false;
                heater = false;
                digitalWrite(heaterPin, LOW);
                isFinish = false;

                timeCounter = 0;
                preHeatTime = 12;

                // clear istTempArray
                for (i = 0; i < 301; i++) {
                  istTemp[i] = 0;
                }
              }
              drawHomeScreen();
              touchedButton = noButton;
            }

            if (touchedButton == buttonSettings) {
              //Serial.println("buttonSettings touched");
              drawSettingsScreen();
            }
            break;
          }
        case sollTempScreen: {
            if (touchedButton == buttonBack) {
              // Serial.println("buttonBack touched");
              drawHomeScreen();
            }
            if (touchedButton == buttonP1) {
              //Serial.println("buttonP1 touched");
              selectedTempPoint = 1;
              drawTempInputScreen();
            }
            if (touchedButton == buttonP2) {
              //Serial.println("buttonP2 touched");
              selectedTempPoint = 2;
              drawTempInputScreen();
            }
            if (touchedButton == buttonP3) {
              //Serial.println("buttonP3 touched");
              selectedTempPoint = 3;
              drawTempInputScreen();
            }
            if (touchedButton == buttonP4) {
              //Serial.println("buttonP4 touched");
              selectedTempPoint = 4;
              drawTempInputScreen();
            }
            if (touchedButton == buttonP5) {
              //Serial.println("buttonP5 touched");
              selectedTempPoint = 5;
              drawTempInputScreen();
            }
            break;
          }
        case tempInputScreen: {
            if (touchedButton == button0) {
              //Serial.println("button0 touched");
              selectedTempPointValue = selectedTempPointValue * 10;
              drawTempPointValueScreen(selectedTempPointValue);

            }
            if (touchedButton == button1) {
              //Serial.println("button1 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 1;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button2) {
              //Serial.println("button2 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 2;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button3) {
              //Serial.println("button3 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 3;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button4) {
              //Serial.println("button4 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 4;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button5) {
              //Serial.println("button5 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 5;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button6) {
              //Serial.println("button6 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 6;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button7) {
              //Serial.println("button7 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 7;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button8) {
              //Serial.println("button8 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 8;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == button9) {
              //Serial.println("button9 touched");
              selectedTempPointValue = selectedTempPointValue * 10 + 9;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == buttonDel) {
              //Serial.println("buttonDel touched");
              selectedTempPointValue = 0;
              drawTempPointValueScreen(selectedTempPointValue);
              //
            }
            if (touchedButton == buttonOK) {
              //Serial.println("buttonOK touched");

              if (actualInputSelection == buttonTime) {
                if (selectedTempPointValue < 0) selectedTempPointValue = 0;
                if (selectedTempPointValue > 300) selectedTempPointValue = 300;

                SollTempPoints[selectedTempPoint].t = selectedTempPointValue;
                // EEPROM.update(selectedTempPoint * 2, SollTempPoints[selectedTempPoint].t);    // update the EEPROM
              }

              else {
                if (selectedTempPointValue < 0) selectedTempPointValue = 0;
                if (selectedTempPointValue > 280) selectedTempPointValue = 280;

                SollTempPoints[selectedTempPoint].T = selectedTempPointValue;
                //EEPROM.update(selectedTempPoint * 2 + 1, SollTempPoints[selectedTempPoint].T);  // update the EEPROM
              }

              drawSollTempScreen();
            }

            if (touchedButton == buttonTemp) {
              //Serial.println("buttonTemp touched");
              actualInputSelection = buttonTemp;
              drawTempInputScreen();
            }
            if (touchedButton == buttonTime) {
              //Serial.println("buttonTime touched");
              actualInputSelection = buttonTime;
              drawTempInputScreen();
            }

            touchedButton = noButton;
            break;
          }

        case settingsScreen: {
            if (touchedButton == buttonBack) {
              //Serial.println("buttonBack touched");
              drawHomeScreen();
            }
            break;
          }

        case noChange: {
            break;
          }
        default: {
            break;
          }
      }
    }
  }
  prevButton = touchedButton;
  delay(50);
}

//--------------------------------------------------------------------------------
/* This function get the actual screen and the touch position and return the touched button

   Input:
    - [screens] currentScreen: The Screen which is currently visible on the TFT.
    - [int16_t] x:             The x-position of the touch.
    - [int16_t] y:             The y-position of the touch.

    Return:
    - [buttons]:               The button which ist touched
*/
buttons getTouchedButton(screens currentScreen, int16_t x, int16_t y ) {
  switch (currentScreen) {
    case homeScreen: {
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
      }

    case sollTempScreen: {
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
      }

    case tempInputScreen: {
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
      }

    case settingsScreen: {
        if (x < 54 && x > 0 && y < 240 && y > 200) {
          return buttonBack;
        }

        else {
          return noButton;
        }
      }

    default: {
        return noButton;
      }
  }
}

/* This funtion draws the ist-temperature line on the screen

   Input:
    - [int16_t] istTemp[]:             ist-temperatur array with the stored temperatur values

    Return:
    - no return
*/
void drawIstTemp(uint16_t istTemp[]) {
  for (i = 0; i < 301; i++) {
    if (istTemp[i] > 0) {
      tft.drawPixel(int16_t(i * 250.0 / 300 + 25), int16_t(186 - istTemp[i] * 156.0 / 300), RED);
    }
  }
}

/* This function draws the actual measured temperature in the middle top. If the heater is on the value has a red background
   Input:
    - [float] actTemp: The actuel measured temperature of the thermocouple sensor

    Return:
    - no return
*/
void drawActualTemp(float actTemp) {
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

/* This function draws the time during preheat
   Input:
    - [uint16_t] remainingTime: The remaining time of preheat process

    Return:
    - no return
*/
void drawRemainingTime(uint16_t remainingTime) {
  tft.fillRect(100, 160, 130, 25, WHITE);
  if (remainingTime > 0) {
    tft.setCursor(100, 165);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.print("preHeat: ");
    tft.print(remainingTime);
  }
}


/* This function draws the home screen

   Input:
    - no inputs

    Return:
    - no return
*/
void drawHomeScreen(void) {
  tft.fillScreen(WHITE);
  tft.setRotation(1);
  tft.drawLine(0, 200, tft.width() - 1, 200, BLACK);
  tft.drawLine(106, 201, 106, tft.height() - 1, BLACK);
  tft.drawLine(213, 201, 213, tft.height() - 1, BLACK);

  tft.setCursor(5, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("SollTemp");

  tft.setCursor(128, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);

  Serial.println(isStarted);
  Serial.println(isFinish);

  if (isStarted == true && isFinish == false) {   //during reflow process
    tft.println("Stop");
  }
  else if (isStarted == false && isFinish == true) {   //end of reflow process
    tft.println("Reset");
  }
  else {
    tft.println("Start");
  }

  tft.setCursor(218, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("Settings");

  tft.drawLine(270, 15, 280, 15, BLUE);     // Label soll
  tft.drawLine(270, 16, 280, 16, BLUE);
  tft.setCursor(282, 13);
  tft.setTextColor(BLUE);  tft.setTextSize(1);
  tft.println("Soll");

  tft.drawLine(270, 25, 280, 25, RED);     // Label ist
  tft.drawLine(270, 26, 280, 26, RED);
  tft.setCursor(282, 23);
  tft.setTextColor(RED);  tft.setTextSize(1);
  tft.println("Ist");

  drawChartAxis();
  drawSollLine(BLUE, false);
  actualScreen = homeScreen;
}

/* This function draws the settings screen

   Input:
    - no inputs

    Return:
    - no return
*/
void drawSettingsScreen(void) {
  tft.fillScreen(WHITE);
  tft.setRotation(1);

  tft.drawLine(0, 200, 54, 200, BLACK);
  tft.drawLine(54, 201, 54, tft.height() - 1, BLACK);

  drawArrow(12, 220);

  tft.setCursor(10, 50);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println(F("Preheat time: 12 sec"));

  tft.setCursor(10, 100);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println(F("Version 1.1"));

  tft.setCursor(10, 150);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println(F("Roman Scheuss"));

  actualScreen = settingsScreen;
}

void drawTempPointValueScreen(int16_t value) {
  tft.fillRect(81, 0, tft.width() - 1, 90, WHITE);
  tft.setCursor(100, 35);
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  tft.println(value);
}

/* This function draw the temperatur input screen

   Input:
    - no inputs

    Return:
    - no return
*/
void drawTempInputScreen(void) {
  int counter = 1;
  tft.fillScreen(WHITE);
  tft.setRotation(1);

  // draw grid
  for (i = 90; i < tft.height() - 1; i += 50) {
    tft.drawLine(0, i, tft.width() - 1, i, BLACK);
  }
  for (i = 160; i < tft.width() - 1; i += 80) {
    tft.drawLine(i, 90, i, tft.height() - 1, BLACK);
  }
  tft.drawLine(80, 0, 80, tft.height() - 1, BLACK);
  tft.drawLine(0, 45, 80, 45, BLACK);


  // draw numbers in grid
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  for (j = 104; j < tft.height() - 1; j += 50) {
    for (i = 33; i < 240; i += 80) {
      tft.setCursor(i, j);
      tft.println(counter);
      counter++;
    }
  }

  tft.setCursor(257, 104);
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  tft.println("Del");

  tft.setCursor(273, 154);
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  tft.println("0");

  tft.setCursor(264, 204);
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  tft.println("OK");

  if (actualInputSelection == buttonTemp) {
    tft.fillRect(0, 0, 80, 45, GREEN);
    selectedTempPointValue = SollTempPoints[selectedTempPoint].T;
  }
  else {
    tft.fillRect(0, 46, 80, 44, GREEN);
    selectedTempPointValue = SollTempPoints[selectedTempPoint].t;
  }

  tft.setCursor(5, 15);
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  tft.println("Temp");

  tft.setCursor(5, 60);
  tft.setTextColor(BLACK);  tft.setTextSize(3);
  tft.println("Time");


  drawTempPointValueScreen(selectedTempPointValue);

  actualScreen = tempInputScreen;
}

/* This function draws the soll-temperature screen

   Input:
    - no inputs

    Return:
    - no return
*/
void drawSollTempScreen(void) {
  tft.fillScreen(WHITE);
  tft.setRotation(1);
  tft.drawLine(0, 200, tft.width() - 1, 200, BLACK);

  tft.drawLine(54, 201, 54, tft.height() - 1, BLACK);
  tft.drawLine(107, 201, 107, tft.height() - 1, BLACK);
  tft.drawLine(161, 201, 161, tft.height() - 1, BLACK);
  tft.drawLine(214, 201, 214, tft.height() - 1, BLACK);
  tft.drawLine(268, 201, 268, tft.height() - 1, BLACK);


  tft.setCursor(70, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("P1");

  tft.setCursor(123, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("P2");

  tft.setCursor(177, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("P3");

  tft.setCursor(230, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("P4");

  tft.setCursor(284, 213);
  tft.setTextColor(BLACK);  tft.setTextSize(2);
  tft.println("P5");

  drawChartAxis();
  drawSollLine(BLUE, true);
  drawArrow(12, 220);
  actualScreen = sollTempScreen;

}

void drawArrow(int16_t x, int16_t y) {
  tft.fillTriangle(x, y, x + 15, y + 10, x + 15, y  - 10, BLACK);
  tft.fillRect(x + 15, y - 5, 15, 11, BLACK);
}



/* This function draws the soll-temperatur line in a existing chart

   Input:
    - [uint16_t] color: The color of the line

    Return:
    - no return
*/
void drawSollLine(uint16_t color, boolean drawIndicators) {
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
    tft.println("P1");
  }

  x1 = int16_t(SollTempPoints[2].t * 25 / 30.0 + xOffsetChart);
  y1 = int16_t(yOffsetChart - SollTempPoints[2].T * 26 / 50.0);
  x2 = int16_t(SollTempPoints[3].t * 25 / 30.0 + xOffsetChart);
  y2 = int16_t(yOffsetChart - SollTempPoints[3].T * 26 / 50.0);
  tft.drawLine(x1, y1, x2, y2, color);

  if (drawIndicators) {
    tft.setCursor(x1 - 10, y1 - 20);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println("P2");
  }

  x1 = int16_t(SollTempPoints[3].t * 25 / 30.0 + xOffsetChart);
  y1 = int16_t(yOffsetChart - SollTempPoints[3].T * 26 / 50.0);
  x2 = int16_t(SollTempPoints[4].t * 25 / 30.0 + xOffsetChart);
  y2 = int16_t(yOffsetChart - SollTempPoints[4].T * 26 / 50.0);
  tft.drawLine(x1, y1, x2, y2, color);

  if (drawIndicators) {
    tft.setCursor(x1 - 10, y1 - 20);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println("P3");
  }

  x1 = int16_t(SollTempPoints[4].t * 25 / 30.0 + xOffsetChart);
  y1 = int16_t(yOffsetChart - SollTempPoints[4].T * 26 / 50.0);
  x2 = int16_t(SollTempPoints[5].t * 25 / 30.0 + xOffsetChart);
  y2 = int16_t(yOffsetChart - SollTempPoints[5].T * 26 / 50.0);
  tft.drawLine(x1, y1, x2, y2, color);

  if (drawIndicators) {
    tft.setCursor(x1 - 10, y1 - 20);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println("P4");
    tft.setCursor(x2 - 10, y2 - 20);
    tft.setTextColor(BLACK);  tft.setTextSize(2);
    tft.println("P5");
  }
}


/* This function calculate the temperature points of the soll-temperatur line with the five soll-temperature points

   Input:
    - no inputs

    Return:
    - no return
*/
void calcSollLine(void) {
  float m = 0;      //gain

  //section one (point0 to point1)
  m = (float(SollTempPoints[1].T - SollTempPoints[0].T)) / (SollTempPoints[1].t - SollTempPoints[0].t);
  for (int i = 0; i <= SollTempPoints[1].t; i++)
  {
    sollTempLine[i] = m * i + SollTempPoints[1].t;
  }

  //section two (point1 to point2)
  m = (float(SollTempPoints[2].T - SollTempPoints[1].T)) / (SollTempPoints[2].t - SollTempPoints[1].t);
  for (int i = (SollTempPoints[1].t + 1); i <= SollTempPoints[2].t; i++)
  {
    sollTempLine[i] = m * (i - SollTempPoints[1].t) + SollTempPoints[1].T;
  }

  //section three (point2 to point3)
  m = (float(SollTempPoints[3].T - SollTempPoints[2].T)) / (SollTempPoints[3].t - SollTempPoints[2].t);
  for (int i = (SollTempPoints[2].t + 1); i <= SollTempPoints[3].t; i++)
  {
    sollTempLine[i] = m * (i - SollTempPoints[2].t) + SollTempPoints[2].T;
  }

  ///section four (point3 to point4)
  m = (float(SollTempPoints[4].T - SollTempPoints[3].T)) / (SollTempPoints[4].t - SollTempPoints[3].t);
  for (int i = (SollTempPoints[3].t + 1); i <= SollTempPoints[4].t; i++)
  {
    sollTempLine[i] = m * (i - SollTempPoints[3].t) + SollTempPoints[3].T;
  }

  ///section five (point4 to point5)
  m = (float(SollTempPoints[5].T - SollTempPoints[4].T)) / (SollTempPoints[5].t - SollTempPoints[4].t);
  for (int i = (SollTempPoints[4].t + 1); i <= SollTempPoints[5].t; i++)
  {
    sollTempLine[i] = m * (i - SollTempPoints[4].t) + SollTempPoints[4].T;
  }
}

/* This function draw the axis of the chart in a existing screen

   Input:
    - no inputs
    Return:
    - not return
*/
void drawChartAxis(void) {
  tft.drawLine(25, 20, 25, 188, BLACK);       // vertical axis
  tft.drawLine(25, 20, 23, 22, BLACK);       // Y arrow
  tft.drawLine(25, 20, 27, 22, BLACK);       // Y arrow
  tft.setCursor(18, 10);
  tft.setTextColor(BLACK);  tft.setTextSize(1);
  tft.println("T[C]");

  tft.drawLine(23, 186, 300, 186, BLACK);     // horizontal axis
  tft.drawLine(298, 184, 300, 186, BLACK);     // X arrow
  tft.drawLine(298, 188, 300, 186, BLACK);     // X arrow
  tft.setCursor(290, 175);
  tft.setTextColor(BLACK);  tft.setTextSize(1);
  tft.println("t[s]");

  // divisionsmark and text X-axis
  int xDivisionMark[xCountDivisionMark] = {25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275};
  const char* xDivisionText[xCountDivisionText] = {" 0 ", " 60", "120", "180", "240", "300"};

  for (i = 0; i < xCountDivisionMark; i++) {
    tft.drawLine(xDivisionMark[i], 186, xDivisionMark[i], 188, BLACK);      // draw division mark X-axis
  }

  for (i = 0; i < xCountDivisionText; i++) {
    tft.setCursor(xDivisionMark[2 * i] - 8, 191);
    tft.setTextColor(BLACK);  tft.setTextSize(1);
    tft.println(xDivisionText[i]);
  }


  // divisionsmark and text Y-axis
  int yDivisionMark[yCountDivisionMark] = {30, 56, 82, 108, 134, 160, 186};
  const char* yDivisionText[yCountDivisionText] = {"300", "250", "200", "150", "100", " 50"};

  for (i = 0; i < yCountDivisionMark; i++) {
    tft.drawLine(23, yDivisionMark[i], 25, yDivisionMark[i], BLACK);      // draw division mark X-axis
  }

  for (i = 0; i < yCountDivisionText; i++) {
    tft.setCursor(3, yDivisionMark[i] - 3);
    tft.setTextColor(BLACK);  tft.setTextSize(1);
    tft.println(yDivisionText[i]);
  }
}

/* This function the interrup service routine for repetitive controlling loop

   Input:
    - no inputs

    Return:
    - no return
*/
void timerIsr()
{
  timeToMeasure = true;
}

