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

uint16_t timeCounter = 0;
uint16_t preHeatTime = 12;

// define default temperature
const uint8_t NUM_OF_SETPOINTS = 6;
Setpoint TemperatureSetpoints[6] = {
  {0,25},
  {30,100},
  {120,150},
  {150,183},
  {210,235},
  {240,183}
};

void evaluateButton(TouchButton::ButtonId touchedButton);
void shiftNeightbourSetpoints(uint8_t setpointIndex);

void setup(void) {
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
    display.drawHomeScreen(processState, TemperatureSetpoints);
    delay(1000);

    //for(i = 0; i < 12; i++){
    // Serial.println(EEPROM.read(i));
    //}
    // for (i = 0; i < 6; i++) {
    // EEPROM.write(2 * i, TemperatureSetpoints[i].time);
    // EEPROM.write(2 * i + 1, TemperatureSetpoints[i].temperature);
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
                if (isTemperature < calcSetpointValue(timeCounter)) {
                        heater = true;
                    digitalWrite(heaterPin, HIGH);
                }
                else {
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                }

                // to terminate a reflow process
                if (timeCounter >= TemperatureSetpoints[5].time) {
                    timeCounter = 0;
                    processState = ProcessState::Finished;
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                    display.drawHomeScreen(processState, TemperatureSetpoints);
                }
                display.drawActualTemperatueChart(timeCounter, isTemperature);

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
                    display.drawSollTempScreen(TemperatureSetpoints);
                } else {
                    //to block buttonSollTemp and buttonSettings during reflow process and before press reset
                    Serial.println(F("Locked -> noAction"));
                }
                break;

            case TouchButton::buttonStartStopReset:
                if (processState == ProcessState::Ready) {
                    // start reflow process
                    processState = ProcessState::Running;
                }
                else {
                    // reset reflow process
                    processState = ProcessState::Ready;
                    heater = false;
                    digitalWrite(heaterPin, LOW);
                    timeCounter = 0;
                    preHeatTime = 12;
                }
                display.drawHomeScreen(processState, TemperatureSetpoints);
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
                display.drawHomeScreen(processState, TemperatureSetpoints);
                break;

            case TouchButton::buttonP1:
            case TouchButton::buttonP2:
            case TouchButton::buttonP3:
            case TouchButton::buttonP4:
            case TouchButton::buttonP5:
                //Serial.println("buttonP1-P5 touched");
                selectedTempPoint = 1 + static_cast<uint8_t>(touchedButton - TouchButton::buttonP1);
                display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, TemperatureSetpoints);
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
                    TemperatureSetpoints[selectedTempPoint].time = selectedTempPointValue;
                    shiftNeightbourSetpoints(selectedTempPoint);
                }
                else {
                    TemperatureSetpoints[selectedTempPoint].temperature = selectedTempPointValue;
                }

                display.drawSollTempScreen(TemperatureSetpoints);
                break;

            case TouchButton::buttonTemp:
                //Serial.println("buttonTemp touched");
                actualInputSelection = TouchButton::buttonTemp;
                display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, TemperatureSetpoints);
                break;

            case TouchButton::buttonTime:
                //Serial.println("buttonTime touched");
                actualInputSelection = TouchButton::buttonTime;
                display.drawTempInputScreen(actualInputSelection == TouchButton::buttonTemp, selectedTempPoint, selectedTempPointValue, TemperatureSetpoints);
                break;

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
    for (uint8_t i=1; i < NUM_OF_SETPOINTS; i++) {
        if (time <= TemperatureSetpoints[i].time) {
            //Serial.print(i);
            //Serial.print(", ");
            //Serial.print(time);
            //Serial.print(", ");
            //Serial.println(map(time, TemperatureSetpoints[i - 1].time, TemperatureSetpoints[i].time, TemperatureSetpoints[i - 1].temperature, TemperatureSetpoints[i].temperature));
            return map(time, TemperatureSetpoints[i-1].time, TemperatureSetpoints[i].time, TemperatureSetpoints[i-1].temperature, TemperatureSetpoints[i].temperature);
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
        if (TemperatureSetpoints[i - 1].time > TemperatureSetpoints[i].time) {
            TemperatureSetpoints[i - 1].time = TemperatureSetpoints[i].time;
        }
    }
    for (uint8_t i=setpointIndex; i < NUM_OF_SETPOINTS-1; i++) {
        if (TemperatureSetpoints[i + 1].time < TemperatureSetpoints[i].time) {
            TemperatureSetpoints[i + 1].time = TemperatureSetpoints[i].time;
        }
    }
}
