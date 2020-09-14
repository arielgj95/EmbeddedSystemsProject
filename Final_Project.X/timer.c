#include "timer.h"

void tmr_setup_period(int n, int ms){
int tckps, pr;
choose_prescaler(ms, &tckps, &pr);
switch (n){
    case TIMER1: {
        TMR1 = 0; // reset the current value;
        T1CONbits.TCKPS = tckps;
        PR1 = pr;
        T1CONbits.TON = 1;
        break;
        }
case TIMER2: {
        TMR2 = 0; // reset the current value;
        T2CONbits.TCKPS = tckps;
        PR2 = pr;
        T2CONbits.TON = 1;
        break;
        }
    }
}

void tmr_wait_period(int n){
switch (n){
    case TIMER1:{
        while (IFS0bits .T1IF == 0) {}
        IFS0bits .T1IF = 0; // set to zero to be able to recognize the next time the timer has expired
        break;
        }
case TIMER2: {
        while (IFS0bits .T2IF == 0) {}
        IFS0bits .T2IF = 0; // set to zero to be able to recognize the next time the timer has expired
        break;
        }
    }
}

void choose_prescaler(int ms, int* tckps, int* pr) {
    //FCY=1843200 Hz ->> 1843,2 clock ticks in 1 ms
    long ticks = 1843.2 * ms; //there can be an approximation
    if (ticks <= 65535) { //if ticks is >65535
        *tckps = 0;
        *pr = ticks;
        return;
    }
    ticks = ticks / 8; //prescaler 1:8
    if (ticks <= 65535) {
        *tckps = 1;
        *pr = ticks;
        return;
    }
    ticks = ticks / 8; //prescaler 1:64
    if (ticks <= 65535) {
        *tckps = 2;
        *pr = ticks;
        return;
    }
    ticks = ticks / 4; //prescaler 1:256
    *tckps = 3;
    *pr = ticks;
    return;
}

void tmr_wait_ms(int n, int ms) {
    tmr_setup_period(n, ms);
    switch (n) {
        case TIMER1:
        {
            while (IFS0bits.T1IF == 0) { //when timer has expired,it goes to 1: I will exit
                //from this loop and then I will set up it again to 0. Doing this, in the
                //next main cycle, I will be again in this loop and wait for an interrupt
                //IEC0bits.T1IE = 0;
            }
            IFS0bits.T1IF = 0;
        }
        case TIMER2:
        {
            while (IFS0bits.T2IF == 0) { //when timer has expired,it goes to 1: I will exit
                //from this loop and then I will set up it again to 0. Doing this, in the
                //next main cycle, I will be again in this loop and wait for an interrupt
                //IEC0bits.T1IE = 0;
            }
            IFS0bits.T2IF = 0;
        }
            //I will exit the above loop only when the timer 1 peripheral has expired
            //and it has set the T1IF flag to one
            //IFS0bits.T1IF = 0; //set to zero to be able to recognize the next time the timer has expired
    }
}