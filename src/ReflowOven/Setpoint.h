#ifndef SETPOINT_H
#define SETPOINT_H

struct Setpoints {
    static const uint8_t NUM_OF_SETPOINTS = 6;

    struct Setpoint {
        uint16_t time;
        uint16_t temperature;
    } sp[NUM_OF_SETPOINTS];
};

#endif // SETPOINT_H