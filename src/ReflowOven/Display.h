#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <string.h>
#include "ProcessState.h"
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
    * @param processState the actual state
    * @param TemperatureSetpoints[] the array of setpoints.
    */
    void drawHomeScreen(ProcessState processState, Setpoint TemperatureSetpoints[]);

    /**
    * This function draws the time during preheat.
    * @param remainingTime The remaining time of preheat process.
    */
    void drawRemainingTime(uint16_t remainingTime);

    /**
    * This funtion draws the actual temperature line on the screen.
    * @param timeCounter time of the measured temperature to display.
    * @param isTemperature actual measured temperature to display.
    */
    void drawActualTemperatueChart(uint16_t time, uint16_t temperature);

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
    void drawTempPointValueScreen(uint16_t value);

    /**
    * Draws the temperature input screen.
    * @param isTempSelected True if actual input selection is temperature.
    * @param selectedTempPoint selected setpoint index.
    * @param selectedTempPointValue the value of the selected setpoint.
    * @param TemperatureSetpoints[] the array of setpoints.
    */
    void drawTempInputScreen(bool isTempSelected, uint8_t selectedTempPoint, uint16_t &selectedTempPointValue, Setpoint TemperatureSetpoints[]);

    /**
    * This function draws the soll-temperature screen.
    * @param TemperatureSetpoints[] the array of setpoints.
    */
    void drawSollTempScreen(Setpoint TemperatureSetpoints[]);


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
    * @param TemperatureSetpoints[] the array of setpoints.
    */
    void drawSollLine(uint16_t color, boolean drawIndicators, Setpoint TemperatureSetpoints[]);

    /**
    * This function draw the axis of the chart in a existing screen.
    * @param TemperatureSetpoints[] the array of setpoints.
    */
    void drawChartAxis(Setpoint TemperatureSetpoints[]);

    /**
    * Draws an arrow at the given positions.
    * @param x x-coordinate of the arrow to draw.
    * @param y y-coordinate of the arrow to draw.
    */
    void drawArrow(int16_t x, int16_t y);

    // constants
    static const int16_t CHART_AREA_X1 = 25;
    static const int16_t CHART_AREA_X2 = 275;
    static const int16_t CHART_AREA_Y1 = 186;
    static const int16_t CHART_AREA_Y2 = 30;
    static const int16_t CHART_WIDTH = CHART_AREA_X2 - CHART_AREA_X1;
    static const uint16_t MAX_TEMPERATURE = 300U;

    // members
    screen actualScreen;
    uint16_t lastChartTemperature;
    uint16_t maxTimeInChart;
};

#endif // DISPLAY_H