// Reflow Oven
//
// Arduino controlled reflow oven built by utilizing a mini oven. It has a TFT
// display with touch control and  thermocouple sensor to control the heating
// elements.
//
// Authors: Roman Scheuss, Marco Graf
// License: This project is licensed under the MIT License - see the LICENSE file for details

#include "ReflowOven.h"
#include "Display.h"
#include "Setpoint.h"
#include "TouchButton.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <TouchScreen.h>
#include <EEPROM.h>
#include <max6675.h>




// generate objects
Display display;
TouchButton touchbutton;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

TouchButton::buttons touchedButton = TouchButton::noButton;
TouchButton::buttons prevButton = TouchButton::noButton;
TouchButton::buttons actualInputSelection = TouchButton::buttonTemp;

uint16_t selectedTempPointValue = 0;
uint8_t selectedTempPoint = 0;

uint32_t lastMeasurementMS;
boolean timeToMeasure = false;

boolean isStarted = false;
boolean isFinish = false;
boolean heater = false;

int i, j;
uint16_t istTemp[301];
uint16_t sollTempLine[301];
uint16_t timeCounter = 0;
uint16_t preHeatTime = 12;

double temp;


// define default temperature
Setpoint SollTempPoints[6] ={
  {0,25},
  {30,100},
  {120,150},
  {150,183},
  {210,235},
  {240,183}
};


void setup(void) {
    // initialize serial communication
    Serial.begin(115200);
    Serial.println(F("Startup... Ready"));

    // clear istTemp array
    for (i = 0; i < 301; i++) {
        istTemp[i] = 0;
    }

    // initialize heater pin
    pinMode(heaterPin, OUTPUT);

    // reset and start display
    display.begin();

    // draw homescreen
    display.drawHomeScreen(isStarted, isFinish, SollTempPoints);
    delay(1000);

    //for(i = 0; i < 12; i++){
    // Serial.println(EEPROM.read(i));
    //}
    // for (i = 0; i < 6; i++) {
    // EEPROM.write(2 * i, SollTempPoints[i].t);
    // EEPROM.write(2 * i + 1, SollTempPoints[i].T);
    // }

    lastMeasurementMS = millis();
}

//--------------------------------------------------------------------------------------------------------
void loop(void) {
    if ((millis() - lastMeasurementMS) >= 1000) {
        lastMeasurementMS += 1000;
        timeToMeasure = true;
    }

    if (timeToMeasure) {

        // read actual temperature
        temp = thermocouple.readCelsius();

        // safety deadlock--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if ((temp > 300) || isnan(temp)) {
            digitalWrite(heaterPin, LOW);
            if (temp > 300) {
                display.drawErrorScreen(F("ERROR: TO HOT!!!"));
            }
            else {
                display.drawErrorScreen(F("FAIL SENSOR!!!"));
            }
            while (1);
        }
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        if (isStarted == true) {

            // to heat up during preheat time
            if (preHeatTime > 0) {
                heater = true;
                digitalWrite(heaterPin, HIGH);
                preHeatTime--;
                display.drawRemainingTime(preHeatTime);
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
                    display.drawHomeScreen(isStarted, isFinish, SollTempPoints);
                }
                display.drawIstTemp(istTemp);
                timeCounter++;
            }
        }

        if ((display.getActualScreen() != Display::tempInputScreen) && (display.getActualScreen() != Display::settingsScreen)) {
            display.drawActualTemp(temp, heater);
        }

        timeToMeasure = false;
    }

    touchedButton = touchbutton.getTouchedButton(display.getActualScreen());

    // limit the pressure force
    if (touchedButton != TouchButton::noButton) {
        //to block buttonSollTemp and buttonSettings during reflow process
        if (isStarted && (touchedButton == TouchButton::buttonSollTemp || touchedButton == TouchButton::buttonSettings)) {
            touchedButton = TouchButton::noButton;
            Serial.println(F("Locked -> noAction"));
        }
        //to block buttonSollTemp and buttonSettings before press reset
        if (isFinish && (touchedButton == TouchButton::buttonSollTemp || touchedButton == TouchButton::buttonSettings)) {
            touchedButton = TouchButton::noButton;
            Serial.println(F("Locked -> noAction"));
        }

        if (prevButton != touchedButton) {

            switch (display.getActualScreen()) {
                case Display::homeScreen:
                    if (touchedButton == TouchButton::buttonSollTemp) {
                        display.drawSollTempScreen(SollTempPoints);
                    }

                    if (touchedButton == TouchButton::buttonStartStopReset) {

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
                        display.drawHomeScreen(isStarted, isFinish, SollTempPoints);
                        touchedButton = TouchButton::noButton;
                    }

                    if (touchedButton == TouchButton::buttonSettings) {
                        //Serial.println("buttonSettings touched");
                        display.drawSettingsScreen();
                    }
                    break;
                case Display::sollTempScreen:
                    if (touchedButton == TouchButton::buttonBack) {
                        // Serial.println("buttonBack touched");
                        display.drawHomeScreen(isStarted, isFinish, SollTempPoints);
                    }
                    if (touchedButton == TouchButton::buttonP1) {
                        //Serial.println("buttonP1 touched");
                        selectedTempPoint = 1;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }
                    if (touchedButton == TouchButton::buttonP2) {
                        //Serial.println("buttonP2 touched");
                        selectedTempPoint = 2;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }
                    if (touchedButton == TouchButton::buttonP3) {
                        //Serial.println("buttonP3 touched");
                        selectedTempPoint = 3;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }
                    if (touchedButton == TouchButton::buttonP4) {
                        //Serial.println("buttonP4 touched");
                        selectedTempPoint = 4;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }
                    if (touchedButton == TouchButton::buttonP5) {
                        //Serial.println("buttonP5 touched");
                        selectedTempPoint = 5;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }
                    break;
                case Display::tempInputScreen:
                    if (touchedButton == TouchButton::button0) {
                        //Serial.println("button0 touched");
                        selectedTempPointValue = selectedTempPointValue * 10;
                        display.drawTempPointValueScreen(selectedTempPointValue);

                    }
                    if (touchedButton == TouchButton::button1) {
                        //Serial.println("button1 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 1;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button2) {
                        //Serial.println("button2 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 2;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button3) {
                        //Serial.println("button3 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 3;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button4) {
                        //Serial.println("button4 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 4;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button5) {
                        //Serial.println("button5 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 5;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button6) {
                        //Serial.println("button6 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 6;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button7) {
                        //Serial.println("button7 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 7;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button8) {
                        //Serial.println("button8 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 8;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::button9) {
                        //Serial.println("button9 touched");
                        selectedTempPointValue = selectedTempPointValue * 10 + 9;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::buttonDel) {
                        //Serial.println("buttonDel touched");
                        selectedTempPointValue = 0;
                        display.drawTempPointValueScreen(selectedTempPointValue);
                        //
                    }
                    if (touchedButton == TouchButton::buttonOK) {
                        //Serial.println("buttonOK touched");

                        if (actualInputSelection == TouchButton::buttonTime) {
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

                        display.drawSollTempScreen(SollTempPoints);
                    }

                    if (touchedButton == TouchButton::buttonTemp) {
                        //Serial.println("buttonTemp touched");
                        actualInputSelection = TouchButton::buttonTemp;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }
                    if (touchedButton == TouchButton::buttonTime) {
                        //Serial.println("buttonTime touched");
                        actualInputSelection = TouchButton::buttonTime;
                        display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                    }

                    touchedButton = TouchButton::noButton;
                    break;

                case Display::settingsScreen:
                    if (touchedButton == TouchButton::buttonBack) {
                        //Serial.println("buttonBack touched");
                        display.drawHomeScreen(isStarted, isFinish, SollTempPoints);
                    }
                    break;

                case Display::noChange:
                    break;
                default:
                    break;
            }
        }
    }
    prevButton = touchedButton;
    delay(50);
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
    for (int i = 0; i <= SollTempPoints[1].t; i++) {
        sollTempLine[i] = m * i + SollTempPoints[1].t;
    }

    //section two (point1 to point2)
    m = (float(SollTempPoints[2].T - SollTempPoints[1].T)) / (SollTempPoints[2].t - SollTempPoints[1].t);
    for (int i = (SollTempPoints[1].t + 1); i <= SollTempPoints[2].t; i++) {
        sollTempLine[i] = m * (i - SollTempPoints[1].t) + SollTempPoints[1].T;
    }

    //section three (point2 to point3)
    m = (float(SollTempPoints[3].T - SollTempPoints[2].T)) / (SollTempPoints[3].t - SollTempPoints[2].t);
    for (int i = (SollTempPoints[2].t + 1); i <= SollTempPoints[3].t; i++) {
        sollTempLine[i] = m * (i - SollTempPoints[2].t) + SollTempPoints[2].T;
    }

    ///section four (point3 to point4)
    m = (float(SollTempPoints[4].T - SollTempPoints[3].T)) / (SollTempPoints[4].t - SollTempPoints[3].t);
    for (int i = (SollTempPoints[3].t + 1); i <= SollTempPoints[4].t; i++) {
        sollTempLine[i] = m * (i - SollTempPoints[3].t) + SollTempPoints[3].T;
    }

    ///section five (point4 to point5)
    m = (float(SollTempPoints[5].T - SollTempPoints[4].T)) / (SollTempPoints[5].t - SollTempPoints[4].t);
    for (int i = (SollTempPoints[4].t + 1); i <= SollTempPoints[5].t; i++) {
        sollTempLine[i] = m * (i - SollTempPoints[4].t) + SollTempPoints[4].T;
    }
}

