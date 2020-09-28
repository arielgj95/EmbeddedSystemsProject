#include "adc.h"
#include "glo.h"
#include <xc.h>

void init_adc(){
    ADCON1bits.SSRC = 7;
    ADCON1bits.ASAM = 1;
    ADCON2bits.CHPS = 0;
    ADCON3bits.SAMC = 15;
    ADCON3bits.ADCS = 63;
    ADCHSbits.CH0NA = 0;
    ADCHSbits.CH0SA = 3;
    ADPCFG = 0xFFFF;
    ADPCFGbits.PCFG3 = 0;
    ADCON1bits.ADON = 1;
}

float acquire_temperature(){
    while (ADCON1bits.DONE == 0);
    int value = ADCBUF0;
    float volt = value / 1024.0 * 5.0;
    float temp = (volt - 0.75)*100 + 25.0;
    return temp;
}