# Arduino-Reflow-Oven
This device controls a pizza oven to produce temperature ramps to use it as a reflow oven. Therefore
several simple modifications are done. The builtin control is disconnected and the heater is controlled
through an opto-SSR (optocoupled solid state relay).

## Requirements
* [Arduino IDE](https://www.arduino.cc/en/main/software)
* Adafruit_GFX library
* SWTFT Library
* TouchScreen Library
* MAX6675 Library
* TimerOne Library
* [Visual Studio](https://visualstudio.microsoft.com/) to have a comfortable IDE
* [Visual Micro](https://www.visualmicro.com/) to integrate Arduino IDE in Visual Studio

## Hardware
The hardware consists of:
* [Pizza oven Clatronic MPO 3520](https://www.clatronic.de/products/de/Elektro-Kleingeraete/Kleine-Backoefen/MPO-3520-Multi-Pizza-Ofen.html)
* [Arduino Uno](https://store.arduino.cc/arduino-uno-rev3)
* [2.4" 240x320 TFT display shield with integrated touch](https://www.aliexpress.com/item/ShengYang-1PCS-LCD-module-TFT-2-4-inch-TFT-LCD-screen-for-Arduino-UNO-R3-Board/32924291239.html)
* [Opto-SSR S202S02 / S202SE2](https://www.aliexpress.com/item/S202S02/32364561514.html)
* [MAX6675 thermocouple module with thermocouple sensor](https://www.aliexpress.com/item/2pcs-lot-MAX6675-K-type-Thermocouple-Temperature-Sensor-Temperature-0-800-Degrees-Module-Free-Shipping-Dropshipping/1843169664.html)

## Housing / Enclosure
The housing for the Arduino Uno and the TFT-Touch-Screen is printed using a 3D-Printer and mounted with a
piece of aluminium to the oven.

## Author
[Marco Graf](https://github.com/grafmar)
Roman Scheuss

## Credits / Attribution
Roman Scheuss built this device and programmed the first version of the arduino code.
