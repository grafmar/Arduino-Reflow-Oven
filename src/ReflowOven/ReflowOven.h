#ifndef REFLOWOVEN_H
#define REFLOWOVEN_H

// ******************************
// Define the hardware: Original(ST7781 TFT) / my setup (SPFD5408 TFT)
//#define USE_ST7781
#define USE_SPFD5408


#ifdef USE_ST7781
// ******************************
// original setup

// TFT display library
#include "SWTFT.h"

#define ROTATION 1

// touchscreen definitons
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

// Touch calibration parameters
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
#define MINPRESSURE 10
#define MAXPRESSURE 1000


#elif defined(USE_SPFD5408)
// ******************************
// my setup

// TFT display library
#include <TftSpfd5408.h> // Hardware-specific library
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define ROTATION 3

// touchscreen definitons
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

// Touch calibration parameters
#define TS_MINX 110
#define TS_MINY 80
#define TS_MAXX 920
#define TS_MAXY 900
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// ******************************
#endif // USE_ST7781


// ******************************
// Pinning of the peripherals

// thermocouple definitons
#define thermoDO 12
#define thermoCS 11
#define thermoCLK 10

// define heater pin
#define heaterPin 19


// ******************************
// display definitons
// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// ******************************

#endif // REFLOWOVEN_H

