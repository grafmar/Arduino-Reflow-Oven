#ifndef EEPROMHANDLER_H
#define EEPROMHANDLER_H

#include "Setpoints.h"
#include <Arduino.h>

class EepromHandler {
public:

    /**
    * Read setpoint-set name from EEPROM.
    * @param index the index of the setpoint entry.
    * @return name of the setpoint-set.
    */
    static String getSetpointSetName(uint8_t index);

    /**
    * Checks if setpoint-set in EEPROM is valid.
    * @param index the index of the setpoint entry.
    * @return true if valid.
    */
    static bool isSetpointSetValid(uint8_t index);

    /**
    * Loads the setpoint-set from EEPROM.
    * @param index the index of the setpoint entry.
    * @param setpointSet the setpoint-set to load into.
    */
    static void loadSetpointSet(uint8_t index, Setpoints& setpointSet);

    /**
    * Saves the given setpoint-set into EEPROM.
    * @param index the index of the setpoint entry.
    * @param setpointSet the setpoint-set to save.
    */
    static void saveSetpointSet(uint8_t index, Setpoints& setpointSet);

private:
    // disallow constructor of pure static class
    EepromHandler();
    ~EepromHandler();
};

#endif // EEPROMHANDLER_H
