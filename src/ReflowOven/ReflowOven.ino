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
#include "ProcessState.h"
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

TouchButton::ButtonId prevButton = TouchButton::noButton;
TouchButton::ButtonId actualInputSelection = TouchButton::buttonTemp;

uint16_t selectedTempPointValue = 0;
uint8_t selectedTempPoint = 0;

uint32_t lastMeasurementMS;
boolean timeToMeasure = false;

ProcessState processState = ProcessState::Ready;

boolean heater = false;

int i, j;
uint16_t sollTempLine[301];
uint16_t timeCounter = 0;
uint16_t preHeatTime = 12;

// define default temperature
Setpoint SollTempPoints[6] ={
  {0,25},
  {30,100},
  {120,150},
  {150,183},
  {210,235},
  {240,183}
};

void evaluateButton(TouchButton::ButtonId touchedButton);

void setup(void) {
    // initialize serial communication
    Serial.begin(115200);
    Serial.println(F("Startup... Ready"));

    // initialize heater pin
    pinMode(heaterPin, OUTPUT);

    // reset and start display
    display.begin();

    // draw homescreen
    display.drawHomeScreen(processState, SollTempPoints);
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
        timeToMeasure = false;

        // read actual temperature
        double temp = thermocouple.readCelsius();

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

        if (processState == ProcessState::Running) {
            // to heat up during preheat time
            if (preHeatTime > 0) {
                heater = true;
                digitalWrite(heaterPin, HIGH);
                preHeatTime--;
                display.drawRemainingTime(preHeatTime);
            }

            // normal heat up
            else {
                uint16_t isTemperature = uint16_t(temp);

                // to turn on and off the heater if necessary
                if (isTemperature < sollTempLine[timeCounter]) {
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
                    processState = ProcessState::Finished;
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                    display.drawHomeScreen(processState, SollTempPoints);
                }
                display.drawIstTemp(timeCounter, isTemperature);
                timeCounter++;
            }
        }

        if ((display.getActualScreen() != Display::tempInputScreen) && (display.getActualScreen() != Display::settingsScreen)) {
            display.drawActualTemp(temp, heater);
        }
    }

    TouchButton::ButtonId touchedButton = touchbutton.getTouchedButton(display.getActualScreen());
    evaluateButton(touchedButton);

    delay(50);
}

void evaluateButton(TouchButton::ButtonId touchedButton) {
    if ((touchedButton != TouchButton::noButton) && (prevButton != touchedButton)) {   // is button pressed?
        switch (touchedButton) {
            case TouchButton::buttonSollTemp:
                if (processState == ProcessState::Ready) {
                    display.drawSollTempScreen(SollTempPoints);
                } else {
                    //to block buttonSollTemp and buttonSettings during reflow process and before press reset
                    Serial.println(F("Locked -> noAction"));
                }
                break;

            case TouchButton::buttonStartStopReset:
                if (processState == ProcessState::Ready) {
                    // start reflow process
                    processState = ProcessState::Running;
                    calcSollLine();
                }
                else {
                    // reset reflow process
                    processState = ProcessState::Ready;
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                    timeCounter = 0;
                    preHeatTime = 12;
                }
                display.drawHomeScreen(processState, SollTempPoints);
                break;

            case TouchButton::buttonSettings:
                if (processState == ProcessState::Ready) {
                    //Serial.println("buttonSettings touched");
                    display.drawSettingsScreen();
                }
                else {
                    //to block buttonSollTemp and buttonSettings during reflow process and before press reset
                    Serial.println(F("Locked -> noAction"));
                }
                break;

            case TouchButton::buttonBack:
                // Display::sollTempScreen or Display::settingsScreen
                // Serial.println("buttonBack touched");
                display.drawHomeScreen(processState, SollTempPoints);
                break;

            case TouchButton::buttonP1:
            case TouchButton::buttonP2:
            case TouchButton::buttonP3:
            case TouchButton::buttonP4:
            case TouchButton::buttonP5:
                //Serial.println("buttonP1-P5 touched");
                selectedTempPoint = 1 + static_cast<uint8_t>(touchedButton - TouchButton::buttonP1);
                display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                break;

            case TouchButton::button0:
            case TouchButton::button1:
            case TouchButton::button2:
            case TouchButton::button3:
            case TouchButton::button4:
            case TouchButton::button5:
            case TouchButton::button6:
            case TouchButton::button7:
            case TouchButton::button8:
            case TouchButton::button9:
                //Serial.println("button0-button9 touched");
                if (selectedTempPointValue < 1000) {
                    selectedTempPointValue = selectedTempPointValue * 10 + static_cast<uint8_t>(touchedButton - TouchButton::button0);
                    display.drawTempPointValueScreen(selectedTempPointValue);
                }
                break;

            case TouchButton::buttonDel:
                //Serial.println("buttonDel touched");
                selectedTempPointValue = selectedTempPointValue / 10;
                display.drawTempPointValueScreen(selectedTempPointValue);
                break;

            case TouchButton::buttonOK:
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
                break;

            case TouchButton::buttonTemp:
                //Serial.println("buttonTemp touched");
                actualInputSelection = TouchButton::buttonTemp;
                display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                break;

            case TouchButton::buttonTime:
                //Serial.println("buttonTime touched");
                actualInputSelection = TouchButton::buttonTime;
                display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, SollTempPoints);
                break;

            default:
                break;
        }
    }

    prevButton = touchedButton;
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

