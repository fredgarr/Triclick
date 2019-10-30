/*
  Triclick

  
  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This code is in the public domain.


*/
// #include </home/frag/Developpement/MyHumanKit/TriClick/consts.h>
#include "Triclick.h"

// constants won't change. They're used here to set pin numbers:
const uint16_t buttonPin = 2;    // the number of the pushbutton pin
const uint16_t ledPin = PIN_A3;    //13  // the number of the LED pin

// Variables will change:
uint16_t buttonState = LOW;       // the current reading from the input pin
uint16_t lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
// Note: milliseconds counter overflows every ~50 days.
uint32_t lastDebounceTime = 0;  // the last time the output pin was toggled
uint32_t debounceDelay = 3;    // the debounce time; 
                                    // EndOfCourse switch bounce time measured to 1ms
                                    // Set to 3 ms for security

clickTable_t click;


// blinkLed: 
// BLinks the led for a given number of time.
// @param[in]  times : Number of blink 
// @param[out] None
void blinkLed(int times) {

    uint16_t k;
    for(k=0; k<times; k++)
    {
        analogWrite(ledPin, 255);
        delay(50);
        analogWrite(ledPin, 0);
        delay(50);
    }
    // Power off the LED at the end:
    analogWrite(ledPin, 0);
}


// debounce: 
// Returns a time value when a stabilized contactor
// modification has been detected. Either 0->1 or 1->0
// @param[in]  timeout : in milliseconds, will return even if no action after maxtime with EXPIRED_TIMER value.
//                       mainly used for calibration part.
// @param[out] Time value
uint32_t debounce(uint16_t timeout)
{
    uint16_t reading = 0;
    uint16_t switchState = False;
    uint32_t endTime = millis() + (uint32_t)timeout;
    uint32_t returnTimer=  0;

    #ifdef DEBUG
    char  message[128];               // for print formating
    sprintf(message, "debounce %d", timeout);
    Serial.println(message);
    #endif

    while(switchState == False)
    {  
        reading = digitalRead(buttonPin);
        // If the switch changed, due to noise or pressing:
        if (reading != lastButtonState) 
        {
            lastDebounceTime = millis();
        }
            
        // FIXME : Manage 50days overflow
        if ((millis() - lastDebounceTime) > debounceDelay) 
        {
            // whatever the reading is at, it's been there for longer than the debounce
            // delay, so take it as the actual current state:

            // if the button state has changed:
            if (reading != buttonState) 
            {
                buttonState = reading;

                // Steady state reached, exit loop
                switchState = True;
                returnTimer = millis();
            }
        }
        // save the reading. Next time through the loop, it'll be the lastButtonState:
        lastButtonState = reading;

        // Check if timout is required
        // if yes and expired, return, FFFFFFFF
        if(timeout != DISABLE_TIMER)
        {
            if(endTime < millis())
            {
                switchState = True;
                returnTimer = EXPIRED_TIMER;
            }
        }
    }
    return returnTimer;
}


// intClick(pClick)
// Initalize click table
// 
void initClick(void *Click)
{
    uint16_t k;
    clickTable_t* pClick;
    pClick = (clickTable_t*)Click;

    for(k=0; k<NUMBER_OF_TRANSITIONS; k++)
    {
        pClick->elem[k].minTime = 0xFFFFFFFF;
        pClick->elem[k].maxTime = 0;
        pClick->elem[k].state = LOW;
    }
}

// printClick(pClick)
// Display click table
// 
void printClick(void* Click)
{
    uint16_t k;
    char message[128];
    clickTable_t* pClick;
    pClick = (clickTable_t*)Click;

    Serial.println(F("Calibration result:"));

    for(k=0; k<NUMBER_OF_TRANSITIONS; k++)
    {
        sprintf((char*)message, "Edge %d: min=%d, max=%d, state=%d", 
                                k, 
                                (uint16_t)pClick->elem[k].minTime,
                                (uint16_t)pClick->elem[k].maxTime,
                                (uint16_t)pClick->elem[k].state);
        Serial.println((char*)message);
    }
}


// doUnitCal(calIdx, duration, pClick)
// will perform 3 clicks recording. Note a check of button not pressed
// must be performed before calling this function
// @param[in]  calIdx: number of current calibrations 
// @param[in]  duration: maximum duration allowed for 3 clicks (in ms)
// @param[in]  pClick: poitner on click table
// @param[out] status : return OK or KO if timeout occured
uint16_t doUnitCal(uint16_t calIdx, uint32_t duration, void* Click)
{
    uint16_t k;
    uint32_t currentTime = 0;
    uint32_t initialTime = 0;
    uint32_t delta_time = 0;
    uint16_t status = True;
    clickTable_t* pClick;
    pClick = (clickTable_t*)Click;
    uint8_t  message[128];               // for print formating

    // Wait for the first clickdown (assuming LOW->HIGH transition):
    initialTime = debounce(DISABLE_TIMER);
    pClick->elem[0].minTime = 0;
    pClick->elem[0].maxTime = 0;
    pClick->elem[0].state = HIGH;
    currentTime = initialTime;

    for(k=1; k< NUMBER_OF_TRANSITIONS; k++)
    {
        
        #ifdef DEBUG
        sprintf((char*)message, "[%d]DebounceTimeout=%d", k,  duration - delta_time);
        Serial.println((char*)message);
        #endif

        currentTime = debounce((uint16_t)(duration - delta_time));
        delta_time = currentTime - initialTime;

        if(currentTime != EXPIRED_TIMER)
        {
            // Record relative time of event and state:
            if(pClick->elem[k].minTime > delta_time)
            {
                #ifdef DEBUG
                sprintf((char*)message, "[%d]Add_minVal=%d", k,  delta_time);
                Serial.println((char*)message);
                #endif
                // If margin too big :
                if(delta_time < TIME_MARGIN)
                {
                    pClick->elem[k].minTime = 0;
                }
                else
                {
                    pClick->elem[k].minTime = delta_time - TIME_MARGIN;
                }
            }
            else if(pClick->elem[k].maxTime < delta_time)
            {
                pClick->elem[k].maxTime = delta_time + TIME_MARGIN;
                #ifdef DEBUG
                sprintf((char*)message, "[%d]Add_maxVal=%d", k,  delta_time);
                Serial.println((char*)message);
                #endif
            }

            // We check if Max is not below Min value (mainly at init round)
            if(pClick->elem[k].maxTime < pClick->elem[k].minTime)
            {
                pClick->elem[k].maxTime = pClick->elem[k].minTime + TIME_MARGIN;
                #ifdef DEBUG
                sprintf((char*)message, "[%d]Add_maxVal=%d", k,  delta_time);
                Serial.println((char*)message);
                #endif
            }


            pClick->elem[k].state = digitalRead(buttonPin);
            #ifdef DEBUG
            sprintf((char*)message, "[%d]Add_Level=%d", k,  digitalRead(buttonPin));
            Serial.println((char*)message);
            #endif
        }
        else
        {
            status = False;
            return status;
        }
    }
    return status;
}



void waitButtonReleased(const char* pMessage)
{
    while(digitalRead(buttonPin)!=LOW)
    {
        debounce(500);
        if(pMessage != NULL)
        {
            Serial.println(pMessage);
        }
    }
    // Reset Global value:
    // FIXME: Use calss instead of global...
    lastButtonState = LOW;
    delay(50);
}

// doCalibration(calTime, duration)
// Performs the calibration of the 3 clicks.
// Will record 'calTime' times the 3 clicks and perform a 
// min/max calculation for each step: High0, Low0, High1, Low1, High2
// and ensure all 3 steps are done in less than 'duration' seconds.
// @param[in]  calTime: number of calibrations 
// @param[in]  duration: maximum duration allowed for 3 clicks (in ms)
// @param[out] None
void doCalibration(uint16_t calTime, uint32_t duration)
{
    uint16_t k;
    uint8_t  message[128];               // for print formating
    uint32_t startupTiming = millis();  // Our base for duration
    uint16_t status = False;

    Serial.println();
    k = 0;
    while(k<calTime)
    {
        waitButtonReleased("Please, release push button to start calibration");

        sprintf((char*)message, "Calibration stage %d/%d", k+1,  calTime);
        Serial.println((char*)message);
        status = doUnitCal(k, duration, &click);
        if(status == False)
        {
            Serial.println(F("TIMING ERROR, TOO LONG. RESTARTING CALIBRATION"));
            k = 0;
            initClick(&click);
        }
        else
        {
            k++;
        } 
    }

    printClick(&click);
}

void doCheckCommand(void *pClick)
{
    clickTable_t* pReferenceClick = (clickTable_t*)pClick;
    uint16_t k;
    uint32_t newTime;
    uint32_t referenceTime;
    uint16_t timeOut;
    uint16_t cmdDetected = 1;
    uint8_t  message[128];               // for print formating

    waitButtonReleased(NULL);

    // Wait first edge, unlimited time:
    referenceTime = debounce(DISABLE_TIMER);
    newTime = referenceTime;

    // Wait for next edge with timeout management
    timeOut = CLICKS_DURATION;

    // Then record next transitions
    for(k=1; k<NUMBER_OF_TRANSITIONS; k++)
    {

        newTime = debounce((timeOut));

        if(EXPIRED_TIMER == newTime)        
        {
            sprintf((char*)message, "TimeOut, reset sequence (%d, %d)", (uint16_t)newTime,timeOut);
            Serial.println((char*)message);
            return;
        }

        timeOut = (uint16_t)(CLICKS_DURATION - newTime + referenceTime);
        newTime -= referenceTime;

        #ifdef DEBUG
        sprintf((char*)message, " stage %d Time = %d, state=%d", k,  (uint16_t)newTime, digitalRead(buttonPin));
        Serial.println((char*)message);
        #endif

        // Check if new edge timing is inside boundaries, and button
        // level is correct
        if( (pReferenceClick->elem[k].minTime < newTime)&&
            (pReferenceClick->elem[k].maxTime > newTime)&&
            (pReferenceClick->elem[k].state == digitalRead(buttonPin)))
        {
            #ifdef DEBUG
            Serial.println(F("Valid Edge"));
            #endif
            cmdDetected++;
        }
    }

    if(cmdDetected == NUMBER_OF_TRANSITIONS)
    {
        Serial.println(F("\nCommand Detected !"));
        // Blink Led LED:
        blinkLed(5);
    }

    Serial.println();

}

// checkOperationMode()
// Will check if a key is pressed during 5 seconds at startup
// if so, enter calibration mode, if not, normal operation
// will start
// @param[in]  None
// @param[out] None
void checkOperationMode()
{
    uint32_t endTime = millis() + SETUP_TIMER; // Initial time
    uint16_t keyPressed = False;

    // Loop for 'SETUP_TIMER' max milliseconds
    while((endTime > millis())&&(keyPressed == False))
    {
        if(Serial.available() > 0)
        {
            char c = Serial.read();
            keyPressed = True;
            Serial.println(F("\nEntering calibration mode"));
        }
        else
        {
            Serial.print('.');
        }
        delay(200);
    }

    if(keyPressed == True)
    {
        doCalibration(CAL_TIME, CLICKS_DURATION);
    }   
}


void setup() {
    // init serial Port for debugging
    Serial.begin(115200);

    // Init pin in/out
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);

    // set initial LED state
    digitalWrite(ledPin, LOW);

    // Initialize click table:
    initClick(&click);

    // Welcome page:
    Serial.println();
    Serial.println(F("TriClick project, press any key to enter calibration mode"));
    Serial.println(F("Switch to normal operation after 5 seconds if no key pressed"));
    
    // Check if entering normal loop or calibration mode:
    checkOperationMode();

    Serial.println(F("Entering normal mode"));
}

void loop() {
    
    // Check commands
    doCheckCommand(&click);

}