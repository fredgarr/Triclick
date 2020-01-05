#ifndef IR_SIEMENS_H
#define IR_SIEMENS_H


// IR_siemens uses pin9 as output (TIMER0) - hardcoded in configure function

#include <Arduino.h>

#define PWM_PIN         9
#define MAX_SWITCH_CODE 63

// Time adjust, required to tune software latency
#define TIME_ADJUST  40

typedef struct
{
    uint16_t short_pulse;
    uint16_t long_pulse;
}button_t;

typedef struct
{
    button_t button1;
    button_t button2;
}command_t;

typedef enum
{
    SHORT_PULSE = 0,
    LONG_PULSE  = 1
}hold_type_t;

typedef enum
{
    BUTTON1 = 0,
    BUTTON2 = 1
}button_type_t;



// Configure pinout and Timer for PWM use to modulate IR LED
void IR_configure();

// Send [SYNC][15 bits data] bit 0 is not sent
void IR_sendData(uint16_t data);

// Send specific SIEMENS code
// <in> switchCode  : from 0x00 to 0x3F, dip switch of the remote controller
// <in> button      : either BUTTON1 or BUTTON2
// <in> holdType    : short hold or long hold of the button (SHORT_PULSE or LONG_PULSE)
void IR_sendCode(uint16_t switchCode, button_type_t button, hold_type_t holdType);

#endif //IR_SIEMENS_H