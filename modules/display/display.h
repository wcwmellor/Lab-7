#ifndef _DISPLAY_H_
#define _DISPLAY_H_

typedef enum {
     DISPLAY_CONNECTION_GPIO_4BITS,
     DISPLAY_CONNECTION_GPIO_8BITS,
     DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER,
} displayConnection_t;

typedef enum {
     DISPLAY_MODE_CHAR, 
     DISPLAY_MODE_GRAPHIC
} displayMode_t;

typedef struct {
   displayConnection_t connection;
} display_t;

void displayInit( displayConnection_t connection );
void displayCharPositionWrite( uint8_t charPositionX, uint8_t charPositionY );
void displayStringWrite( const char * str );
void displayClear( void );
void displayModeWrite( displayMode_t mode );
void displayBitmapWrite( uint8_t* bitmap );

#endif // _DISPLAY_H_
