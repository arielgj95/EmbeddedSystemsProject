#include "adc.h"
#include "glo.h"
#include <xc.h>

void init_adc(){
    ADCON1bits.SSRC = 7; //sampling fixed duration
    ADCON1bits.ASAM = 1; //automatic sampling start
    ADCON2bits.CHPS = 0; // Channel 0
    ADCON3bits.SAMC = 15; // 15 Tad
    ADCON3bits.ADCS = 63; // Tad == 32 * Tcy
    ADCHSbits.CH0NA = 0; //Input - CH0 = GND
    ADCHSbits.CH0SA = 3; //Input + CH0 = AN3
    ADPCFG = 0xFFFF;
    ADPCFGbits.PCFG3 = 0;
    ADCON1bits.ADON = 1; // Turn on ADC
}

float acquire_temperature(){
    while (ADCON1bits.DONE == 0); //wait for the conversion 
    int value = ADCBUF0; //read sensor value
    //These formulas come from the fact We want convert 0-5 Volt using 1024 levels
    float volt = value / 1024.0 * 5.0;
    //from volt to celsius 
    float temp = (volt - 0.75)*100 + 25.0;
    return temp;
}