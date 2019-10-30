#ifndef TRICLICK_H
#define TRICLICK_H


// #define DEBUG

// Functional constants:
//--------------------------------------------
// 5 seconds before going to normal operation
#define SETUP_TIMER 5000 

// Number of calibrations:
#define CAL_TIME 4

// Maximum time window duration authorized for the N clicks (in ms)
// Time margin 50ms
// FIXME: Some *2 clock error requiering to divide timer by 2...
//        should be 3000 to get 3 seconds
#define CLICKS_DURATION 3000
#define TIME_MARGIN  150

// Max number of cliks authorised per time window
#define NUMBER_OF_CLICKS 3
#define NUMBER_OF_TRANSITIONS     (NUMBER_OF_CLICKS*2)

// Debounce timer expire return
#define EXPIRED_TIMER 0xFFFFFFFF
#define DISABLE_TIMER 0xFFFF


// Click definition:
typedef struct click_T
{
    uint32_t minTime;
    uint32_t maxTime;
    uint16_t state; // High or Low
}click_t;

// Define the click table, as rising and follwing edges of button 
//  Note that first index should always be [0, HIGH]
typedef struct clickTable_T
{
    click_t elem[NUMBER_OF_TRANSITIONS];
}clickTable_t;

// Misc:
//--------------------------------------------
#define True 1
#define False 0

typedef unsigned long uint32_t;
typedef long int32_t;
typedef unsigned int uint16_t;
typedef int int16_t;
typedef unsigned char uint8_t;

#endif //TRICLICK_H