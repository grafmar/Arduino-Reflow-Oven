#ifndef SETPOINTS_H
#define SETPOINTS_H
#include <Arduino.h>

struct Setpoints {
    static const uint8_t NUM_OF_SETPOINTS = 6;
    static const uint8_t NAME_LENGTH = 40;

    struct Setpoint {
        uint16_t time;
        uint16_t temperature;
    } sp[NUM_OF_SETPOINTS];
    char name[NAME_LENGTH];
};

#endif // SETPOINTS_H
