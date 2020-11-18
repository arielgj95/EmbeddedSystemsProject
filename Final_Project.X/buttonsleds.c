#include<xc.h>
#include"buttonled1.h"
#include "glo.h"
#include "timer.h"
#include "spi.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void init_leds(){
    TRISBbits.TRISB0 = 0; //LED D3 output
    TRISBbits.TRISB1 = 0; //LED D4 output
    LATBbits.LATB0 = 0; //LED D3 logic level low
    LATBbits.LATB1 = 0; // LED D4 logic level low
}

void init_buttons(){
    TRISEbits.TRISE8 = 1; //enable S5 button as input
    TRISDbits.TRISD0 = 1; //enable S6 button as input
    IEC0bits.INT0IE = 1; // enable interrupt for INT0
    IEC1bits.INT1IE = 1; // enable interrupt for INT1
    IEC0bits.T2IE = 1; // enable interrupt for T2
    IEC1bits.T4IE = 1; // enable interrupt for T4
}

void __attribute__ (( __interrupt__ , __auto_psv__)) _INT0Interrupt(){
IFS0bits.INT0IF = 0; //clear the S5 interrupt flag
status = 2; // set safe mode
//motors are stopped immediatly
n1 = 0; 
n2 = 0;
pdc(&pdc1, &pdc2, t_pwm, n1, n2);
PDC1 = pdc1;
PDC2 = pdc2;
IEC0bits.INT0IE = 0; // disable interrupt for INT0
tmr_setup_period(TIMER2, 20); // start timer 2
}

void __attribute__ (( __interrupt__ , __auto_psv__ )) _T2Interrupt(){ //this is use for the bouncing problem of S5 button
IFS0bits.T2IF = 0; //clear the T2 interrupt flag
T2CONbits.TON = 0; // stop timer 2
IFS0bits.INT0IF = 0; // reset INT0 IF
IEC0bits.INT0IE = 1; // enable interrupt for INT0
}

void __attribute__ (( __interrupt__ , __auto_psv__)) _INT1Interrupt(){ 
IFS1bits.INT1IF = 0; //clear the S6 interrupt flag 
print = !print; //change the state of print state
count_print = 0; //when S6 is pressed, the long string restart from the beginning 
IEC1bits.INT1IE = 0; // disable interrupt for INT1
tmr_setup_period(TIMER4, 20); // start timer 4
}

void __attribute__ (( __interrupt__ , __auto_psv__ )) _T4Interrupt(){ //this is used for the bouncing problem of S6 button
IFS1bits.T4IF = 0; //clear the T4 interrupt flag
T4CONbits.TON = 0; // stop timer 4
IFS1bits.INT1IF = 0; // reset INT1 IF
IEC1bits.INT1IE = 1; // enable interrupt for INT1
}