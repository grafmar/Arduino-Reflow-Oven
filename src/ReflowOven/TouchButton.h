#ifndef TOUCHBUTTON_H
#define TOUCHBUTTON_H
#include <TouchScreen.h>
#include "Display.h"

class TouchButton {
public:
    enum ButtonId {
        buttonSettings,
        buttonAboutInfo,
        buttonBack,
        buttonEdit,
        buttonLoad,
        buttonSave,
        buttonBackSettings,
        buttonP1,
        buttonP2,
        buttonP3,
        buttonP4,
        buttonP5,
        button0,
        button1,
        button2,
        button3,
        button4,
        button5,
        button6,
        button7,
        button8,
        button9,
        buttonDel,
        buttonOK,
        buttonStartStopReset,
        buttonTime,
        buttonTemp,
        buttonDefault,
        buttonM1,
        buttonM2,
        buttonM3,
        buttonM4,
        buttonM5,
        buttonSaveAs,
        noButton
    };

    struct TouchButtonElement {
        Display::screen screen;
        //uint16_t screen;
        uint16_t x1;
        uint16_t x2;
        uint16_t y1;
        uint16_t y2;
        ButtonId buttonId;
//        uint16_t buttonId;
    };

    static const uint16_t NUM_OF_TOUCH_BUTTONS = 44;
    static const PROGMEM TouchButtonElement TOUCH_BUTTONS[NUM_OF_TOUCH_BUTTONS];

    TouchButton();
    ~TouchButton();

    /**
    * Evaluates the touched area and returns the touched button.
    * @param currentScreen The Screen which is currently visible on the TFT.
    * @return the touched button. 'noButton' is returned if no button is pressed.
    */
    TouchButton::ButtonId getTouchedButton(Display::screen currentScreen);

    /**
    * Evaluates the touched key of the display keyboard.
    * @return the touched key or 0 if no key is pressed.
    */
    char getTouchedKey();

    /**
    * Checks if is touched.
    * @return true if touched.
    */
    bool isTouched();

private:

    // members
    TouchScreen ts;
};

#endif // TOUCHBUTTON_H
