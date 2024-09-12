#ifndef _WIPER_MODE_SELECTOR_H_
#define _WIPER_MODE_SELECTOR_H_


// declaration of public data types
typedef enum{
    OFF_MODE,
    LOW_MODE,
    INT_MODE,
    HIGH_MODE,
} selector_mode_t;


// declaration of functions
selector_mode_t selectorUpdate(selector_mode_t selectorMode);


#endif
