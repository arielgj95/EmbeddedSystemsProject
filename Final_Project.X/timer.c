#include "timer.h"
#include <xc.h>
#include "glo.h"


void tmr_setup_period(int n, int ms){ //create a period of ms using timer n 
int tckps, pr;
choose_prescaler(ms, &tckps, &pr);
switch (n){
    case TIMER1: {
        TMR1 = 0; // reset the current value of the timer count register;
        T1CONbits.TCKPS = tckps; //choose the prescaler
        PR1 = pr; //put in PR1 the value of the clocks that I will have in some ms (passed to the function)
        T1CONbits.TON = 1; //start the timer
        break;
        }
case TIMER2: {
        TMR2 = 0; // reset the current value;
        T2CONbits.TCKPS = tckps;
        PR2 = pr;
        T2CONbits.TON = 1;
        break;
        }
    case TIMER3:{;
        TMR3 = 0; //reset the current value
        T3CONbits.TCKPS = tckps;
        PR3 = pr;
        T3CONbits.TON = 1;
        break;
        }
    case TIMER4:{
        TMR4 = 0; //reset the current value
        T4CONbits.TCKPS = tckps;
        PR4 = pr;
        T4CONbits.TON = 1;
        break;
        }
    }
}

void tmr_wait_period(int n){ //function used to wait from a certain moment to the end of a choosen timer (n) period
switch (n){
    case TIMER1:{
        while (IFS0bits.T1IF == 0) {} //wait until the interrupt flag goes to 1 (if it goes to 1,the timer has expired)
        IFS0bits.T1IF = 0; // set to zero to be able to recognize the next time the timer has expired
        break;
        }
case TIMER2: {
        while (IFS0bits.T2IF == 0) {}
        IFS0bits.T2IF = 0; 
        break;
        }
    case TIMER3:{
        while (IFS0bits.T3IF == 0){}
        IFS0bits.T3IF = 0; 
        break;
        }
    case TIMER4:{
        while (IFS1bits.T4IF == 0){}
        IFS1bits.T4IF = 0; 
        break;;
        }
    }
}

void choose_prescaler(int ms, int* tckps, int* pr) { //This function choose the best prescaler depending on ms
    //FCY=1843200 Hz ->> 1843,2 clock ticks in 1 ms
    long ticks = 1843.2 * ms; //there can be an approximation, count how many ticks I have in ms (passed to the function)
    if (ticks <= 65535) { //if ticks is >65535
        *tckps = 0; //choose the first prescaler
        *pr = ticks; //and put the ticks in the variable pr
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

void tmr_wait_ms(int n, int ms) { //function used to wait some ms from the moment that I call this function
    tmr_setup_period(n, ms); // setup a period of ms in the choosen tiimer n
    switch (n) {
        case TIMER1:
        {
            while (IFS0bits.T1IF == 0) { //when timer has expired,it goes to 1: I will exit
                //from this loop and then I will set up it again to 0. Doing this, in the
                //next main cycle, I will be again in this loop and wait for an interrupt
                //IEC0bits.T1IE = 0;
            }
            IFS0bits.T1IF = 0; //put the flag again to 0
            break;
        }
        case TIMER2:
        {
            while (IFS0bits.T2IF == 0) {
            }
            IFS0bits.T2IF = 0;
            break;
        }
        case TIMER3:
        {
            while (IFS0bits.T3IF == 0) { 
            }
            IFS0bits.T3IF = 0;
            break;
        }    
        case TIMER4:
        {
            while (IFS1bits.T4IF == 0) { 
            }
            IFS1bits.T4IF = 0;
            break;
        }
        //I will exit the above loop only when the timer 1 peripheral has expired
            //and it has set the T1IF flag to one
            //IFS0bits.T1IF = 0; //set to zero to be able to recognize the next time the timer has expired
    }
}
