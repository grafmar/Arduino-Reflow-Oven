#include "EepromHandler.h"
#include <EEPROM.h>

String EepromHandler::getSetpointSetName(uint8_t index) {
    if (isSetpointSetValid(index)) {
        Setpoints tempSetpoint;
        const uint16_t address = index * sizeof(Setpoints);
        EEPROM.get(address, tempSetpoint);
        return String(tempSetpoint.name);
    }
    else {
        return "<EMPTY>";
    }
}

bool EepromHandler::isSetpointSetValid(uint8_t index) {
    Setpoints tempSetpoint;
    const uint16_t address = index * sizeof(Setpoints);
    if (address + sizeof(Setpoints) > EEPROM.length()) {
        return false;
    }

    EEPROM.get(address, tempSetpoint);
    if ((tempSetpoint.sp[0].time != 0U) || (tempSetpoint.sp[0].temperature != 25U)) {
        return false;
    }

    return true;
}

void EepromHandler::loadSetpointSet(uint8_t index, Setpoints & setpointSet) {
    const uint16_t address = index * sizeof(Setpoints);
    if (isSetpointSetValid(index)) {
        EEPROM.get(address, setpointSet);
    } else {
        // define default temperature setpoints
        setpointSet.sp[0] = { 0, 25 };
        setpointSet.sp[1] = { 30,100 };
        setpointSet.sp[2] = { 120,150 };
        setpointSet.sp[3] = { 150,183 };
        setpointSet.sp[4] = { 210,235 };
        setpointSet.sp[5] = { 240,183 };
    }

    if (address == 0U) {
        // reset name for default setpoints
        memset(setpointSet.name, 0U, setpointSet.NAME_LENGTH);
    }
}

void EepromHandler::saveSetpointSet(uint8_t index, Setpoints & setpointSet) {
    const uint16_t address = index * sizeof(Setpoints);
    if (address + sizeof(Setpoints) > EEPROM.length()) {
        return;
    }
    EEPROM.put(address, setpointSet);
}
