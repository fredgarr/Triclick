#include "IR_siemens.h"



command_t codes[MAX_SWITCH_CODE + 1]={
                        0x1FCC,0x1FE8,0x1F82,0x1FE4,
                        0x1F48,0x1F6C,0x1F06,0x1F60,
                        0x1ECE,0x1EEA,0x1E80,0x1EE6,
                        0x1E4A,0x1E6E,0x1E04,0x1E62,
                        0x1DC8,0x1DEC,0x1D86,0x1DE0,
                        0x1D4C,0x1D68,0x1D02,0x1D64,
                        0x1CCA,0x1CEE,0x1C84,0x1CE2,
                        0x1C4E,0x1C6A,0x1C00,0x1C66,
                        0x1B80,0x1BE6,0x1Bce,0x1Bea,
                        0x1B04,0x1B62,0x1B4a,0x1B6e,
                        0x1A82,0x1AE4,0x1ACC,0x1AE8,
                        0x1A06,0x1A60,0x1A48,0x1A6C,
                        0x1984,0x19E2,0x19CA,0x19EE,
                        0x1900,0x1966,0x194E,0x196A,
                        0x18C8,0x18EC,0x1886,0x18E0,
                        0x184C,0x1868,0x1802,0x1864,
                        0x1786,0x17E0,0x17C8,0x17EC,
                        0x1702,0x1764,0x174C,0x1768,
                        0x1684,0x16E2,0x16CA,0x16EE,
                        0x1600,0x1666,0x164E,0x166A,
                        0x1582,0x15E4,0x15CC,0x15E8,
                        0x1506,0x1560,0x1548,0x156C,
                        0x14CE,0x14EA,0x1480,0x14E6,
                        0x1404,0x1462,0x144A,0x146E,
                        0x1384,0x13E2,0x13CA,0x13EE,
                        0x1300,0x1366,0x134E,0x136A,
                        0x12C8,0x12EC,0x1286,0x12E0,
                        0x124C,0x1268,0x1202,0x1264,
                        0x11CE,0x11EA,0x1180,0x11E6,
                        0x114A,0x116E,0x1104,0x1162,
                        0x10CC,0x10E8,0x1082,0x10E4,
                        0x1048,0x106C,0x1006,0x1060,
                        0x0FCE,0x0FEA,0x0F80,0x0Fe6,
                        0x0F4A,0x0F6E,0x0F04,0x0F62,
                        0x0ECC,0x0EE8,0x0E82,0x0EE4,
                        0x0E48,0x0E6C,0x0E06,0x0E60,
                        0x0DCA,0x0DEE,0x0D84,0x0DE2,
                        0x0D4E,0x0D6A,0x0D00,0x0D66,
                        0x0CC8,0x0CEC,0x0C86,0x0CE0,
                        0x0C4C,0x0C68,0x0C02,0x0C64,
                        0x0BCC,0x0BE8,0x0B82,0x0BE4,
                        0x0B48,0x0B6C,0x0B06,0x0B60,
                        0x0ACE,0x0AEA,0x0A80,0x0AE6,
                        0x0A4A,0x0A6E,0x0A04,0x0A62,
                        0x09C8,0x09EC,0x0986,0x09E0,
                        0x094C,0x0968,0x0902,0x0964,
                        0x08CA,0x08EE,0x0842,0x08E2,
                        0x084E,0x086A,0x0800,0x0866,
                        0x07CA,0x07EE,0x0784,0x07E2,
                        0x074E,0x076A,0x0700,0x0766,
                        0x06C8,0x06EC,0x0686,0x06E0,
                        0x064C,0x0668,0x0602,0x0664,
                        0x05CE,0x05EA,0x0580,0x05E6,
                        0x054A,0x056E,0x0504,0x0562,
                        0x04CC,0x04E8,0x0482,0x04E4,
                        0x0448,0x046C,0x0406,0x0460,
                        0x03C8,0x03EC,0x0386,0x03E0,
                        0x034C,0x0368,0x0302,0x0364,
                        0x02CA,0x02EE,0x0284,0x02E2,
                        0x024E,0x026A,0x0200,0x0266,
                        0x01CC,0x01E8,0x0182,0x01E4,
                        0x0148,0x016C,0x0106,0x0160,
                        0x00CE,0x00EA,0x0080,0x00E6,
                        0x004A,0x006E,0x0004,0x0062
                    };


volatile byte *TIMSK = {&TIMSK1};
volatile byte *TCCRnA = {&TCCR1A};
volatile byte *TCCRnB = {&TCCR1B};
volatile unsigned int *OCRnA = {&OCR1A};
volatile unsigned int *OCRnB = {&OCR1B};
volatile unsigned int *ICRn	= {&ICR1};


void IR_configure()
{

    pinMode(PWM_PIN,OUTPUT);

    /*
    TCCR1A & TCCR1B registers setting:
    WGM1 3..0   = 1 1 1 0 => Fast PWM Mode, Counter will increase until it matches ICR1. 
    COM1A 1..0  = 1 1     => Set OC1A/OC1B on Compare Match, clear OC1A/OC1B at BOTTOM (inverting mode)
    CS1 2..0    = 0 0 1   => clk I/O /1 (No prescaling)
    ICNC1       = 0       => Disable Input noise canceller
    ICES1       = 0       => Disable Capture Edge 

    ICR1        = 35     => Counter TOP value, will give a PWM freq of 16MHz/36 ~ 457kHz
    
    OCR1A       = ICR1/2 => 50% duty cycle

    TIMSK1      = 00000000 => ICIE1=0 No interrupt,            OCIE1B=0 No Compare B interrupt,
                              OCIE1A=0 No Compare A interrupt, TOIE1=0  No overflow interrupt
    */
    noInterrupts();
    
    *ICRn = 35;
    *TCCRnA = _BV(WGM11) | _BV(COM1A1)  | _BV(COM1B0) | _BV(COM1B1); 
    *TCCRnB = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
    *TIMSK = 0;

    *OCRnA = *ICRn>>1;
    *OCRnB = 0;

    interrupts();
}

void pwmCtrl(uint16_t ctrl)
{
    if(true == ctrl)
    {
        // Enable PWM output
        *TCCRnA |= _BV(COM1A1);
    }
    else
    {
        // Disable PWM output
        *TCCRnA &= 0x3F;
    }
}

void sendPulse()
{
    uint16_t bitSwitch = 1;

    pwmCtrl(bitSwitch);
    delayMicroseconds(50);
    pwmCtrl(~bitSwitch);
}


void sendSync()
{
    sendPulse();
    delayMicroseconds(2500 - TIME_ADJUST);
    sendPulse();
    delayMicroseconds(995 - TIME_ADJUST);
    sendPulse();
    delayMicroseconds(995 - TIME_ADJUST);
    sendPulse();
    delayMicroseconds(995 - TIME_ADJUST);
    sendPulse();

    delayMicroseconds(1945 - TIME_ADJUST);
    sendPulse();
}

void IR_sendData(uint16_t data)
{
    uint16_t bit = 0;

    sendSync();

    for(uint16_t k=15; k>0; k--)
    {
        bit = (data >> k) & 0x0001;
        if(1 == bit)
        {
            delayMicroseconds(1500 - TIME_ADJUST);
            sendPulse();
        }
        else
        {
            delayMicroseconds(1000 - TIME_ADJUST);
            sendPulse();
        }
    }
}


void IR_sendCode(uint16_t switchCode, button_type_t button, hold_type_t holdType)
 {

    if (switchCode > MAX_SWITCH_CODE)
    {
        return;
    }
    
    switch (button)
    {
        case BUTTON1:
            if(SHORT_PULSE == holdType)
            {
                IR_sendData(codes[MAX_SWITCH_CODE - switchCode].button1.short_pulse);
            }
            else
            {
                IR_sendData(codes[MAX_SWITCH_CODE - switchCode].button1.long_pulse);
            }         

        case BUTTON2:
            if(SHORT_PULSE == holdType)
            {
                IR_sendData(codes[MAX_SWITCH_CODE - switchCode].button2.short_pulse);
            }
            else
            {
                IR_sendData(codes[MAX_SWITCH_CODE - switchCode].button2.long_pulse);
            }         

        default:
            ; // Do nothing
    }
 }
