#ifndef TOUCHBUTTON_H
#define TOUCHBUTTON_H
#include <TouchScreen.h>
#include "Display.h"

class TouchButton {
public:
    enum buttons {
        buttonSollTemp,
        buttonSettings,
        buttonBack,
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
        noButton
    };

    TouchButton();
    ~TouchButton();

    /**
    * Evaluates the touched area and returns the touched button.
    * @param currentScreen The Screen which is currently visible on the TFT.
    * @return the touched button. 'noButton' is returned if no button is pressed.
    */
    TouchButton::buttons getTouchedButton(Display::screen currentScreen);

private:

    // members
    TouchScreen ts;
};

#endif // TOUCHBUTTON_H
