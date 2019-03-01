#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <string.h>
#include "ProcessState.h"
#include "Setpoints.h"


class Display {
public:
    /**
    * The different screens
    */
    enum screen {
        homeScreen,
        settingsScreen,
        editSetpointsScreen,
        setpointInputScreen,
        loadSetpointsScreen,
        saveSetpointsScreen,
        enterNameScreen,
        aboutInfoScreen,
        noChange
    };

    /**
    * Constructor.
    * @param setpoints struct of setpoints
    * @param processState the actual state
    */
    Display(Setpoints& setpoints, ProcessState& processState);

    /** Destructor. */
    ~Display();

    /**
    * Reset and initialize the TFT display
    */
    void begin();

    /**
    * This function draws the home screen
    */
    void drawHomeScreen();

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
    void drawAboutInfoScreen(void);

    /**
    * Draws the actual temperatue value on top of the screen.
    * @param value temperature value to display.
    */
    void drawTempPointValueScreen(uint16_t value);

    /**
    * This function draws the settings screen.
    */
    void drawSettingsScreen(void);

    /**
    * This function draws the edit setpoints screen.
    */
    void drawEditSetpointsScreen();

    /**
    * Draws the temperature input screen.
    * @param isTempSelected True if actual input selection is temperature.
    * @param selectedTempPoint selected setpoint index.
    * @param selectedTempPointValue the value of the selected setpoint.
    */
    void drawSetpointInputScreen(bool isTempSelected, uint8_t selectedTempPoint, uint16_t &selectedTempPointValue);

    /**
    * Draws screen for loading setpoints from EEPROM.
    */
    void drawLoadSetpointsScreen();

    /**
    * Draws screen for saving setpoints to EEPROM.
    */
    void drawSaveSetpointsScreen();

    /**
    * Draws screen for entering the setpoint-set name to store into EEPROM.
    * @param name the name to display first
    */
    void drawEnterNameScreen(String name);

    /**
    * Update the name on the enterNameScreen.
    * @param name the name to display first
    */
    void updateNameValue(String name);

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
    */
    void drawSollLine(uint16_t color, boolean drawIndicators);

    /**
    * This function draw the axis of the chart in a existing screen.
    */
    void drawChartAxis();

    /**
    * Draws an arrow at the given positions.
    * @param x x-coordinate of the arrow to draw.
    * @param y y-coordinate of the arrow to draw.
    */
    void drawArrow(int16_t x, int16_t y);

    /**
    * Draws an arrow at the given positions.
    * @param x x-coordinate of the arrow to draw.
    * @param y y-coordinate of the arrow to draw.
    */
    void drawButtons(screen screenId);

    // constants
    static const int16_t CHART_AREA_X1 = 25;
    static const int16_t CHART_AREA_X2 = 275;
    static const int16_t CHART_AREA_Y1 = 186;
    static const int16_t CHART_AREA_Y2 = 30;
    static const int16_t CHART_WIDTH = CHART_AREA_X2 - CHART_AREA_X1;
    static const uint16_t MAX_TEMPERATURE = 300U;

    // members
    Setpoints& m_setpoints;
    ProcessState& m_processState;
    screen actualScreen;
    uint16_t lastChartTemperature;
    uint16_t maxTimeInChart;
};

#endif // DISPLAY_H