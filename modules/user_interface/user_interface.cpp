//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

#include "user_interface.h"

#include "code.h"
#include "siren.h"
#include "smart_home_system.h"
#include "fire_alarm.h"
#include "date_and_time.h"
#include "temperature_sensor.h"
#include "gas_sensor.h"
#include "matrix_keypad.h"
#include "display.h"
#include "GLCD_fire_alarm.h"
#include "motor.h"
#include "gate.h"
#include "pc_serial_com.h"
//=====[Declaration of private defines]========================================

#define DISPLAY_REFRESH_TIME_REPORT_MS 1000
#define DISPLAY_REFRESH_TIME_ALARM_MS 300

//=====[Declaration of private data types]=====================================

typedef enum{
    DISPLAY_ALARM_STATE,
    DISPLAY_REPORT_STATE
} displayState_t;

//=====[Declaration and initialization of public global objects]===============

DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

InterruptIn gateOpenButton(PF_9);
InterruptIn gateCloseButton(PF_8);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

char codeSequenceFromUserInterface[CODE_NUMBER_OF_KEYS];

//=====[Declaration and initialization of private global variables]============

static displayState_t displayState = DISPLAY_REPORT_STATE;
static int displayAlarmGraphicSequence = 0;
static int displayRefreshTimeMs = DISPLAY_REFRESH_TIME_REPORT_MS;

static bool incorrectCodeState = OFF;
static bool systemBlockedState = OFF;

static bool codeComplete = false;
static int numberOfCodeChars = 0;

//=====[Declarations (prototypes) of private functions]========================

static void userInterfaceMatrixKeypadUpdate();
static void incorrectCodeIndicatorUpdate();
static void systemBlockedIndicatorUpdate();

static void userInterfaceDisplayInit();
static void userInterfaceDisplayUpdate();
static void userInterfaceDisplayReportStateInit();
static void userInterfaceDisplayReportStateUpdate();
static void userInterfaceDisplayAlarmStateInit();
static void userInterfaceDisplayAlarmStateUpdate();

static void gateOpenButtonCallback();
static void gateCloseButtonCallback();

//=====[Implementations of public functions]===================================

void userInterfaceInit()
{
    gateOpenButton.mode(PullUp);
    gateCloseButton.mode(PullUp);

    gateOpenButton.fall(&gateOpenButtonCallback);
    gateCloseButton.fall(&gateCloseButtonCallback);
    
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
    matrixKeypadInit( SYSTEM_TIME_INCREMENT_MS );
    userInterfaceDisplayInit();
}

void userInterfaceUpdate()
{
    userInterfaceMatrixKeypadUpdate();
    incorrectCodeIndicatorUpdate();
    systemBlockedIndicatorUpdate();
    userInterfaceDisplayUpdate();
}

bool incorrectCodeStateRead()
{
    return incorrectCodeState;
}

void incorrectCodeStateWrite( bool state )
{
    incorrectCodeState = state;
}

bool systemBlockedStateRead()
{
    return systemBlockedState;
}

void systemBlockedStateWrite( bool state )
{
    systemBlockedState = state;
}

bool userInterfaceCodeCompleteRead()
{
    return codeComplete;
}

void userInterfaceCodeCompleteWrite( bool state )
{
    codeComplete = state;
}

//=====[Implementations of private functions]==================================

static void userInterfaceMatrixKeypadUpdate()
{
    static int numberOfHashKeyReleased = 0;
    char keyReleased = matrixKeypadUpdate();

    if( keyReleased != '\0' ) {

        switch( keyReleased ) {
            case '1':
                gateOpen();
                displayCharPositionWrite(5, 3);
                displayStringWrite("Opening ");
                pcSerialComStringWrite("Keypad: Gate opening\r\n");
            break;

            case '2':
                gateClose();
                displayCharPositionWrite(5, 3);
                displayStringWrite("Closing ");
                pcSerialComStringWrite("Keypad: Gate closing\r\n");
            break;

            case '3':
                motorDirectionWrite(STOPPED);
                displayCharPositionWrite(5, 3);
                displayStringWrite("Stopped ");
                pcSerialComStringWrite("Keypad: Gate stopped\r\n");
            break;

            default:
            break;
        }

        if( sirenStateRead() && !systemBlockedStateRead() ) {
            if( !incorrectCodeStateRead() ) {
                codeSequenceFromUserInterface[numberOfCodeChars] = keyReleased;
                numberOfCodeChars++;
                if ( numberOfCodeChars >= CODE_NUMBER_OF_KEYS ) {
                    codeComplete = true;
                    numberOfCodeChars = 0;
                }
            } else {
                if( keyReleased == '#' ) {
                    numberOfHashKeyReleased++;
                    if( numberOfHashKeyReleased >= 2 ) {
                        numberOfHashKeyReleased = 0;
                        numberOfCodeChars = 0;
                        codeComplete = false;
                        incorrectCodeState = OFF;
                    }
                }
            }
        }
    }
}

static void userInterfaceDisplayReportStateInit()
{
    displayState = DISPLAY_REPORT_STATE;
    displayRefreshTimeMs = DISPLAY_REFRESH_TIME_REPORT_MS;

    displayCharPositionWrite( 0, 0 );
    displayStringWrite( "Temperature:" );

    displayCharPositionWrite( 0, 1 );
    displayStringWrite( "Gas:" );
    
    displayCharPositionWrite( 0, 2 );
    displayStringWrite( "Alarm:" );

    displayCharPositionWrite( 0, 3 );
    displayStringWrite( "Gate:" );
}

static void userInterfaceDisplayReportStateUpdate()
{
    char temperatureString[3] = "";
    
    sprintf(temperatureString, "%.0f", temperatureSensorReadCelsius());
    displayCharPositionWrite(12, 0);
    displayStringWrite(temperatureString);
    displayCharPositionWrite(14, 0);
    displayStringWrite("'C");

    displayCharPositionWrite(4, 1);
    if (gasDetectorStateRead()) {
        displayStringWrite("Detected    ");
    } else {
        displayStringWrite("Not Detected");
    }

    displayCharPositionWrite(6, 2);
    if (sirenStateRead()) {
        displayStringWrite("ON ");
    } else {
        displayStringWrite("OFF");
    }

    displayCharPositionWrite(5, 3);
    switch (gateStatusRead()) {
        case GATE_CLOSED:   displayStringWrite("Closed  "); break;
        case GATE_OPEN:     displayStringWrite("Open    "); break;
        case GATE_OPENING:  displayStringWrite("Opening "); break;
        case GATE_CLOSING:  displayStringWrite("Closing "); break;
    }
}

static void userInterfaceDisplayAlarmStateInit()
{
    displayState = DISPLAY_ALARM_STATE;
    displayRefreshTimeMs = DISPLAY_REFRESH_TIME_ALARM_MS;

    displayClear();

    displayModeWrite( DISPLAY_MODE_GRAPHIC );
   
    displayAlarmGraphicSequence = 0;
}

static void userInterfaceDisplayAlarmStateUpdate()
{
    switch( displayAlarmGraphicSequence ){
        case 0:
            displayBitmapWrite( GLCD_fire_alarm[0] );
            displayAlarmGraphicSequence++;
        break;
        case 1:
            displayBitmapWrite( GLCD_fire_alarm[1] );
            displayAlarmGraphicSequence++;
        break;
        case 2:
            displayBitmapWrite( GLCD_fire_alarm[2] );
            displayAlarmGraphicSequence++;
        break;
        case 3:
            displayBitmapWrite( GLCD_fire_alarm[3] );
            displayAlarmGraphicSequence = 0;
        break;
        default:
            displayBitmapWrite( GLCD_ClearScreen );
            displayAlarmGraphicSequence = 1;
        break;                   
    }
}
static void userInterfaceDisplayInit()
{
    displayInit( DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER );
    userInterfaceDisplayReportStateInit();
}

static void userInterfaceDisplayUpdate()
{
    static int accumulatedDisplayTime = 0;
    static gateStatus_t lastGateStatus = GATE_CLOSED;
    
    if( accumulatedDisplayTime >= displayRefreshTimeMs ) {
        accumulatedDisplayTime = 0;

        char temperatureString[3] = "";
        sprintf(temperatureString, "%.0f", temperatureSensorReadCelsius());
        displayCharPositionWrite(12, 0);
        displayStringWrite(temperatureString);
        displayCharPositionWrite(14, 0);
        displayStringWrite("'C");

        displayCharPositionWrite(4, 1);
        if (gasDetectorStateRead()) {
            displayStringWrite("Detected    ");
        } else {
            displayStringWrite("Not Detected");
        }

        displayCharPositionWrite(6, 2);
        if (sirenStateRead()) {
            displayStringWrite("ON !");
        } else {
            displayStringWrite("OFF ");
        }

        displayCharPositionWrite(5, 3);
        switch (gateStatusRead()) {
            case GATE_CLOSED:   displayStringWrite("Closed  "); break;
            case GATE_OPEN:     displayStringWrite("Open    "); break;
            case GATE_OPENING:  displayStringWrite("Opening "); break;
            case GATE_CLOSING:  displayStringWrite("Closing "); break;
        }

        if( gateStatusRead() != lastGateStatus ) {
            lastGateStatus = gateStatusRead();
            switch( lastGateStatus ) {
                case GATE_CLOSED:   pcSerialComStringWrite("Gate status: Closed\r\n");   break;
                case GATE_OPEN:     pcSerialComStringWrite("Gate status: Open\r\n");     break;
                case GATE_OPENING:  pcSerialComStringWrite("Gate status: Opening\r\n");  break;
                case GATE_CLOSING:  pcSerialComStringWrite("Gate status: Closing\r\n");  break;
            }
        }

    } else {
        accumulatedDisplayTime =
            accumulatedDisplayTime + SYSTEM_TIME_INCREMENT_MS;        
    }
}
static void incorrectCodeIndicatorUpdate()
{
    incorrectCodeLed = incorrectCodeStateRead();
}

static void systemBlockedIndicatorUpdate()
{
    systemBlockedLed = systemBlockedState;
}

static void gateOpenButtonCallback()
{
    gateOpen();
}

static void gateCloseButtonCallback()
{
    gateClose();
}

static void gateCloseButtonCallback()
{
    gateClose();
}
