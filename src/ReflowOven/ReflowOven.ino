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
#include "EepromHandler.h"
#include "ProcessState.h"
#include "Setpoints.h"
#include "TouchButton.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <TouchScreen.h>
#include <EEPROM.h>
#include <max6675.h>


// generate objects
Setpoints setpoints;
ProcessState processState = ProcessState::Ready;

Display display(setpoints, processState);
TouchButton touchbutton;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

TouchButton::ButtonId prevButton = TouchButton::noButton;
TouchButton::ButtonId actualInputSelection = TouchButton::buttonTemp;

uint16_t selectedTempPointValue = 0;
uint8_t selectedTempPoint = 0;

uint32_t lastMeasurementMS;
boolean timeToMeasure = false;

boolean heater = false;

uint16_t timeCounter = 0;
uint16_t preHeatTime = 12;
uint8_t saveIndex = 0;
String saveName = "";

void evaluateButton(TouchButton::ButtonId touchedButton);
void shiftNeightbourSetpoints(uint8_t setpointIndex);

void setup(void) {
    EepromHandler::loadSetpointSet(0, setpoints);   // loads hard
    if (!EepromHandler::isSetpointSetValid(0)) {
        EepromHandler::loadSetpointSet(0, setpoints);
    }

    // initialize serial communication
    Serial.begin(115200);
    Serial.print(F("ReflowOven Version V"));
    Serial.print(VERSION_MAJOR);
    Serial.print(F("."));
    Serial.println(VERSION_MINOR);

    // initialize heater pin
    pinMode(heaterPin, OUTPUT);

    // reset and start display
    display.begin();

    // draw homescreen
    display.drawHomeScreen();
    delay(1000);

    // EEPROM readout
    //for (uint16_t address=0; address < EEPROM.length(); address++) {
    //    if (address % 16 == 0) {
    //        Serial.println();
    //        char buffer[10];
    //        sprintf(buffer, "%04d:", address);
    //        Serial.print(buffer);
    //    }
    //    Serial.print(" ");
    //    Serial.print(EEPROM.read(address), HEX);
    //}

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
                if (isTemperature < calcSetpointValue(timeCounter)) {
                        heater = true;
                    digitalWrite(heaterPin, HIGH);
                }
                else {
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                }

                // to terminate a reflow process
                if (timeCounter >= setpoints.sp[5].time) {
                    timeCounter = 0;
                    processState = ProcessState::Finished;
                    Serial.println(F("Finished"));
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                    display.drawHomeScreen();
                }
                display.drawActualTemperatueChart(timeCounter, isTemperature);

                timeCounter++;
            }
        }

        if (display.getActualScreen() == Display::homeScreen) {
            display.drawActualTemp(temp, heater);
        }
    }
    
    if (display.getActualScreen() == Display::enterNameScreen) {
        char key = touchbutton.getTouchedKey();
        if (key != 0) {
            if (key == 127) {
                saveName = saveName.substring(0, saveName.length() - 1);
            } else {
                if (saveName.length() < 21) {
                    saveName += key;
                }
            }
            display.updateNameValue(saveName);
        }
    }

    TouchButton::ButtonId touchedButton = touchbutton.getTouchedButton(display.getActualScreen());
    evaluateButton(touchedButton);

    while (touchbutton.isTouched()) {}
    delay(50);
}

void evaluateButton(TouchButton::ButtonId touchedButton) {
    if ((touchedButton != TouchButton::noButton) && (prevButton != touchedButton)) {   // is button pressed?
        switch (touchedButton) {
            case TouchButton::buttonBackSettings:   // fall through
            case TouchButton::buttonSettings:
                if (processState == ProcessState::Ready) {
                    display.drawSettingsScreen();
                } else {
                    //to block buttonSettings and buttonAboutInfo during reflow process and before press reset
                    Serial.println(F("Locked -> noAction"));
                }
                break;

            case TouchButton::buttonStartStopReset:
                if (processState == ProcessState::Ready) {
                    // start reflow process
                    processState = ProcessState::Running;
                    Serial.println(F("Running"));
                }
                else {
                    // reset reflow process
                    processState = ProcessState::Ready;
                    Serial.println(F("Ready"));
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                    timeCounter = 0;
                    preHeatTime = 12;
                }
                display.drawHomeScreen();
                break;

            case TouchButton::buttonAboutInfo:
                if (processState == ProcessState::Ready) {
                    //Serial.println("buttonAboutInfo touched");
                    display.drawAboutInfoScreen();
                }
                else {
                    //to block buttonSettings and buttonAboutInfo during reflow process and before press reset
                    Serial.println(F("Locked -> noAction"));
                }
                break;

            case TouchButton::buttonBack:
                // Display::editSetpointsScreen or Display::aboutInfoScreen
                // Serial.println("buttonBack touched");
                display.drawHomeScreen();
                break;

            case TouchButton::buttonEdit:
                display.drawEditSetpointsScreen();
                break;

            case TouchButton::buttonLoad:
                display.drawLoadSetpointsScreen();
                break;

            case TouchButton::buttonSave:
                display.drawSaveSetpointsScreen();
                break;

            case TouchButton::buttonP1:
            case TouchButton::buttonP2:
            case TouchButton::buttonP3:
            case TouchButton::buttonP4:
            case TouchButton::buttonP5:
                //Serial.println("buttonP1-P5 touched");
                selectedTempPoint = 1 + static_cast<uint8_t>(touchedButton - TouchButton::buttonP1);
                display.drawSetpointInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue);
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
            {
                //Serial.println("button0-button9 touched");
                uint32_t value = selectedTempPointValue;
                value = value * 10U + static_cast<uint8_t>(touchedButton - TouchButton::button0);

                if (((value <= MAX_SETPOINT_TIME) && (actualInputSelection == TouchButton::buttonTime))  
                    || ((value <= MAX_SETPOINT_TEMPERATURE) && (actualInputSelection == TouchButton::buttonTemp))) {

                    selectedTempPointValue = static_cast<uint16_t>(value);
                    display.drawTempPointValueScreen(selectedTempPointValue);
                }
                break;
            }
            case TouchButton::buttonDel:
                //Serial.println("buttonDel touched");
                selectedTempPointValue = selectedTempPointValue / 10U;
                display.drawTempPointValueScreen(selectedTempPointValue);
                break;

            case TouchButton::buttonOK:
                //Serial.println("buttonOK touched");
                if (actualInputSelection == TouchButton::buttonTime) {
                    if (selectedTempPointValue < 1U) {
                        selectedTempPointValue = 1U;
                    }
                    setpoints.sp[selectedTempPoint].time = selectedTempPointValue;
                    shiftNeightbourSetpoints(selectedTempPoint);
                }
                else {
                    setpoints.sp[selectedTempPoint].temperature = selectedTempPointValue;
                }

                display.drawEditSetpointsScreen();
                break;

            case TouchButton::buttonTemp:
                //Serial.println("buttonTemp touched");
                actualInputSelection = TouchButton::buttonTemp;
                display.drawSetpointInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue);
                break;

            case TouchButton::buttonTime:
                //Serial.println("buttonTime touched");
                actualInputSelection = TouchButton::buttonTime;
                display.drawSetpointInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue);
                break;

            case TouchButton::buttonDefault:
            case TouchButton::buttonM1:
            case TouchButton::buttonM2:
            case TouchButton::buttonM3:
            case TouchButton::buttonM4:
            case TouchButton::buttonM5:
            {
                uint8_t index = static_cast<uint8_t>(touchedButton - TouchButton::buttonDefault);
                if (display.getActualScreen() == Display::loadSetpointsScreen) {
                    if (EepromHandler::isSetpointSetValid(index)) {
                        EepromHandler::loadSetpointSet(index, setpoints);
                        display.drawSettingsScreen();
                    }
                } else {
                    saveIndex = index;
                    saveName = setpoints.name;
                    display.drawEnterNameScreen(saveName);
                }
                break;
            }

            case TouchButton::buttonSaveAs:
                memcpy(setpoints.name, saveName.c_str(), saveName.length());
                EepromHandler::saveSetpointSet(saveIndex, setpoints);
                display.drawSettingsScreen();
                break;

            case TouchButton::noButton:
            default:
                break;
        }
    }

    prevButton = touchedButton;
}


/**
* This function calculates the interpolated temperature setpoint at the given time.
* @param time the time value to calculate the temperatue setpoint of.
* @return the calculated temperature setpoint.
*/
uint16_t calcSetpointValue(uint16_t time) {
    for (uint8_t i=1; i < Setpoints::NUM_OF_SETPOINTS; i++) {
        if (time <= setpoints.sp[i].time) {
            //Serial.print(i);
            //Serial.print(", ");
            //Serial.print(time);
            //Serial.print(", ");
            //Serial.println(map(time, setpoints.sp[i - 1].time, setpoints.sp[i].time, setpoints.sp[i - 1].temperature, setpoints.sp[i].temperature));
            return map(time, setpoints.sp[i-1].time, setpoints.sp[i].time, setpoints.sp[i-1].temperature, setpoints.sp[i].temperature);
        }
    }
    return 0; // just in case we get here
}


/**
* Shifts the time of the neighbouring setpoints so the time of the setpoints does not overrung each other (time will be in ascending order).
* @param setpointIndex the index of the adjusted setpoint.
*/
void shiftNeightbourSetpoints(uint8_t setpointIndex) {
    for (uint8_t i=setpointIndex; i > 0; i--) {
        if (setpoints.sp[i - 1].time > setpoints.sp[i].time) {
            setpoints.sp[i - 1].time = setpoints.sp[i].time;
        }
    }
    for (uint8_t i=setpointIndex; i < Setpoints::NUM_OF_SETPOINTS-1; i++) {
        if (setpoints.sp[i + 1].time < setpoints.sp[i].time) {
            setpoints.sp[i + 1].time = setpoints.sp[i].time;
        }
    }
}
