#include "mbed.h"
#include "arm_book_lib.h"


#include "wiper_mode_selector.h"
// declaration of macors




// declaration of private objects
AnalogIn modeSelector(A0);


// declration of private data types


// declaration of functions
selector_mode_t selectorUpdate(selector_mode_t selectorMode){
    if(0 <= modeSelector.read() <= 0.25){
        selectorMode = OFF_MODE;
    }
    if(0.26 <= modeSelector.read() <= 0.5){
        selectorMode = LOW_MODE;
    }
    if(0.51 <= modeSelector.read() <= 0.75){
        selectorMode = INT_MODE;
    }
    if(0.76 <= modeSelector.read()){
        selectorMode = HIGH_MODE;
    }
    return selectorMode;
}
