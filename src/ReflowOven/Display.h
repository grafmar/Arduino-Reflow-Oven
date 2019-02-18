#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <string.h>
#include "Setpoint.h"


class Display {
public:
    /**
    * The different screens
    */
    enum screen {
        homeScreen,
        sollTempScreen,
        tempInputScreen,
        settingsScreen,
        noChange
    };

    /** Constructor. */
    Display();
    /** Destructor. */
    ~Display();

    /**
    * Reset and initialize the TFT display
    */
    void begin();

    /**
    * This function draws the home screen
    * @param isStarted true if started
    * @param isFinish true if finished
    * @param SollTempPoints[] the array of setpoints.
    */
    void drawHomeScreen(bool isStarted, bool isFinish, Setpoint SollTempPoints[]);

    /**
    * This function draws the time during preheat.
    * @param remainingTime The remaining time of preheat process.
    */
    void drawRemainingTime(uint16_t remainingTime);

    /**
    * This funtion draws the ist-temperature line on the screen.
    * @param istTemp ist-temperatur array with the stored temperatur values.
    */
    void drawIstTemp(uint16_t istTemp[]);

    /**
    * This function draws the actual measured temperature in the middle top.
    * If the heater is on the value has a red background.
    * @param actTemp The actuel measured temperature of the thermocouple sensor.
    * @param heater true if heater is active.
    */
    void drawActualTemp(float actTemp, bool heater);

    /**
    * This function draws the settings screen.
    */
    void drawSettingsScreen(void);

    /**
    * Draws the actual temperatue value on top of the screen.
    * @param value temperature value to display.
    */
    void drawTempPointValueScreen(int16_t value);

    /**
    * Draws the temperature input screen.
    * @param isTempSelected True if actual input selection is temperature.
    * @param selectedTempPoint selected setpoint index.
    * @param selectedTempPointValue the value of the selected setpoint.
    * @param SollTempPoints[] the array of setpoints.
    */
    void drawTempInputScreen(bool isTempSelected, uint8_t selectedTempPoint, uint16_t &selectedTempPointValue, Setpoint SollTempPoints[]);

    /**
    * This function draws the soll-temperature screen.
    * @param SollTempPoints[] the array of setpoints.
    */
    void drawSollTempScreen(Setpoint SollTempPoints[]);


    /**
    * Draws an error screen.
    */
    void drawErrorScreen(String message);

    /**
    * Gets the actual screen.
    * @return the actual screen.
    */
    screen getActualScreen();

private:

    /**
    * This function draws the soll-temperatur line in a existing chart.
    * @param color The color of the line.
    * @param drawIndicators if true, indicators are drawn.
    * @param SollTempPoints[] the array of setpoints.
    */
    void drawSollLine(uint16_t color, boolean drawIndicators, Setpoint SollTempPoints[]);

    /**
    * This function draw the axis of the chart in a existing screen.
    * @param SollTempPoints[] the array of setpoints.
    */
    void drawChartAxis(Setpoint SollTempPoints[]);

    /**
    * Draws an arrow at the given positions.
    * @param x x-coordinate of the arrow to draw.
    * @param y y-coordinate of the arrow to draw.
    */
    void drawArrow(int16_t x, int16_t y);

    // members
    screen actualScreen;
};

#endif // DISPLAY_H