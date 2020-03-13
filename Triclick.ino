/* 
MIT License

Copyright (c) 2019 fredgarr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Original code: Debounce.ino
modified 8 May 2014  by Scott Fitzgerald   modified 2 Sep 2016
by Arturo Guadalupi   modified 8 Sep 2016   by Colby Newman
This code is in the public domain.

*/
#define TESTING

#include "Triclick.h"
#include "IR_siemens.h"

// constants won't change. They're used here to set pin numbers:
const uint16_t buttonPin = 2;    // the number of the pushbutton pin
const uint16_t ledPin = PIN_A3;  // 13 - the number of the LED pin
const uint16_t relayPin = 8;     // Mechanical 5V relayPin

// Variables will change:
uint16_t buttonState = LOW;       // the current reading from the input pin
uint16_t lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
// Note: milliseconds counter overflows every ~50 days.
uint32_t lastDebounceTime = 0;  // the last time the output pin was toggled

uint32_t timeClick_tbl[NUMBER_OF_CLICKS];
uint16_t currentClickOffset = 0;
uint16_t numberOfClick = 0;
uint16_t logFlag = 0;


// For testing purpose only, store all the clicks in FLASH memory
// Maximum data is 512 clicks (leading to 2048 bytes)
#ifdef TESTING
    #include <EEPROM.h>
    uint16_t dataToSaveOffset = 4;

    void writeEEPROM(uint8_t type, uint32_t time)
    {
        // Save current value pointer
        EEPROM.write(0, (uint8_t)(dataToSaveOffset & 0xFF));
        EEPROM.write(1, (uint8_t)((dataToSaveOffset >> 8) & 0xFF));

        // Save Type and 24 MS-bits of time, little Endian
        EEPROM.write(dataToSaveOffset++, type);
        EEPROM.write(dataToSaveOffset++, (uint8_t)((time >> 8)& 0xFF));
        EEPROM.write(dataToSaveOffset++, (uint8_t)((time >> 16)& 0xFF));
        EEPROM.write(dataToSaveOffset++, (uint8_t)((time >> 24)& 0xFF));
        if(dataToSaveOffset == EEPROM.length())
        {
            dataToSaveOffset = EEPROM.length()-4;
        }
    }


    void resetEEPROMptr()
    {
        dataToSaveOffset = 4;
        // Save current value pointer
        EEPROM.write(0, (uint8_t)(dataToSaveOffset & 0xFF));
        EEPROM.write(1, (uint8_t)((dataToSaveOffset >> 8) & 0xFF));

    }

    uint16_t readEEPROM(uint16_t offset)
    {
        uint16_t data;

        data = (uint16_t)EEPROM.read(offset);
        data += ((uint16_t)EEPROM.read(offset+1))<<8;
        return data;
    }


    void readTestingTable()
    {
        char message[128];
        uint16_t dataToSaveOffset;
        uint8_t  flag;
        uint32_t timer;

        memset(message, 0, 128);
        snprintf(message, 127, "EEPROM Len: %d", EEPROM.length());
        Serial.println(message);

        // Get save pointer :
        dataToSaveOffset = readEEPROM(0);

        for(uint16_t k=4; k<=(dataToSaveOffset); k+=4)
        {

            memset(message, 0, 64);
            flag = EEPROM.read(k);
            timer = ((uint32_t)(EEPROM.read(k+1)))<<8;
            timer += ((uint32_t)(EEPROM.read(k+2)))<<16;
            timer += ((uint32_t)(EEPROM.read(k+3)))<<24;
            snprintf(message, 63, "offset %d : [%d, %ld]", k/4, flag, timer);
            Serial.println(message);
        }
    }


    void logClick(uint8_t type, uint32_t time)
    {
            writeEEPROM(type, time);
    }
#else
    void logClick(uint8_t type, uint32_t time){}
#endif

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
// @param[out] Time value
uint32_t debounce()
{
    uint16_t reading = 0;
    uint16_t switchState = False;
    uint32_t returnTimer=  0;


    while(switchState == False)
    {  
        reading = digitalRead(buttonPin);
        // If the switch changed, due to noise or pressing:
        if (reading != lastButtonState) 
        {
            lastDebounceTime = millis();
        }
            
        if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) 
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
    }
    return returnTimer;
}


// printClick(pClick)
// Display click table
// 
void printClick(uint16_t offset)
{
    uint16_t k;
    char message[128];

    for(k=0; k<NUMBER_OF_CLICKS; k++)
    {
        if(k == offset)
        {
            sprintf((char*)message, "D%d = %x-%x <==", k, (uint16_t)(timeClick_tbl[k]>>16), (uint16_t)timeClick_tbl[k]);
        }
        else
        {
            sprintf((char*)message, "D%d = %x-%x", k, (uint16_t)(timeClick_tbl[k]>>16), (uint16_t)timeClick_tbl[k]);
        }
        
        Serial.println((char*)message);
    }
    Serial.println();
}


void waitButtonReleased()
{
    while(digitalRead(buttonPin)!=LOW)
    {
        debounce();
    }
}



void setup() {
    uint16_t k;

    // Init pin in/out
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(relayPin,OUTPUT);

    // Configure IR output port & PWM
    IR_configure();

    // set initial LED state
    digitalWrite(ledPin, LOW);

    // set initial Relay pin state
    digitalWrite(relayPin, LOW);

    // Reset delta time table:
    for (k=0; k<NUMBER_OF_CLICKS; k++)
    {
        timeClick_tbl[k] = EXPIRED_TIMER;
    }

    blinkLed(10);

#ifdef TESTING
    {
        // init serial Port for debugging
        Serial.begin(115200);

        uint32_t endTime = millis() + SETUP_TIMER; // Wait for 10 seconds
        uint16_t keyPressed = False;
        Serial.println(F("Wait 10sec before printing logged data..."));

        // Loop for 'SETUP_TIMER' max milliseconds
        while((endTime > millis())&&(keyPressed == False))
        {
            if(Serial.available() > 0)
            {
                char c = Serial.read();
                keyPressed = True;
                Serial.println(F("Display data and reset EEPROM"));
            }
            else
            {
                Serial.print('.');
            }
            delay(200);
        }    
        if(keyPressed == True)
        {
            readTestingTable();
            resetEEPROMptr();

        }
        else
        {
            Serial.println(F("Do not reset EEPROM pointer"));
            dataToSaveOffset = readEEPROM(0);
            // Mark New cession
            writeEEPROM(0xFA, 0x00FAFAFA);
        }

    }
#endif

    Serial.println(F("Init done, entering normal mode"));

    // Wait for the button to be released befor loop starts:
    waitButtonReleased();
}


// Check all Delta-Time between last and first click
uint16_t CheckDeltaTime()
{
    uint16_t offset = currentClickOffset;
    uint32_t totalTime;
    uint16_t result = False;

    // Get offset of first click:
    // End of the table if first element, or 
    // next element otherwise
    if(offset == NUMBER_OF_CLICKS-1)
    {
        offset = 0;
    }
    else 
    {
        offset ++;
    }

    // Compute (LastClick - FirstClick):
    totalTime = timeClick_tbl[currentClickOffset] - timeClick_tbl[offset];

    if( totalTime < CLICKS_DURATION)
    {
        result = true;
    }

    return result;
}


void send_IR_cmd()
{
    // Secure the call: 
    // sed on the Two buttons short and long commands for CODE 0x02
    // Repeate 3 times (~ 10 sec)
    for(uint16_t k=0; k<3; k++)
    {
        IR_sendCode(0x02, BUTTON1, SHORT_PULSE);
        blinkLed(5);
        delay(100);
        IR_sendCode(0x02, BUTTON1, LONG_PULSE);
        blinkLed(5);
        delay(100);
        IR_sendCode(0x02, BUTTON2, SHORT_PULSE);
        blinkLed(5);
        delay(100);
        IR_sendCode(0x02, BUTTON2, LONG_PULSE);
        blinkLed(5);
        delay(100);
    }

}


void loop() {

    // Wait for a rising edge (button pressed)
    // and store timer when rising edge occurs.
    timeClick_tbl[currentClickOffset] = debounce();
    // Wait for the button to be released 
    waitButtonReleased();

    numberOfClick++;
    blinkLed(1);
    
    // Calculate the delta between timer 
    // If ok, blink the led
    if(numberOfClick == NUMBER_OF_CLICKS)
    {    
        if(CheckDeltaTime())
        {
            numberOfClick = 0;
            logFlag = 1;

            // Double command between relay and IR :
            digitalWrite(relayPin, HIGH);
            send_IR_cmd();
            digitalWrite(relayPin, LOW);
        }
        else
        {
            numberOfClick --;
        }  
    }

    logClick(logFlag, timeClick_tbl[currentClickOffset]);
    logFlag = 0;
    
    // Increment table offset (loop to 0)
    if(currentClickOffset == NUMBER_OF_CLICKS-1)
    {
        currentClickOffset = 0;
    }
    else
    {
        currentClickOffset ++;
    }
}