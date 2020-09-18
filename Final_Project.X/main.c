// Francesco Bruno & Ariel Gjaci 


// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FPR = XT                 // Primary Oscillator Mode (XT)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "parser.h"
#include "pwm.h"
#include "spi.h"
#include "uart.h"
#include "timer.h"
 
#define MAX_RPM 8000
#define MIN_RPM -8000
#define MAXTASKS 3

typedef struct {//one struct for one task to be scheduled
    int n;//counter
    int N;//when n=N, the task have to be executed 
} heartbeat;


//GLOBAL VARIABLES
long int Fosc = 7372800; // 7.3728 MHz
long int Fcy = (Fosc / 4.0); // number of clocks in one second = 1,843,200 clocks for each second
int *pdc1 = 0;
int *pdc2 = 0;
heartbeat schedInfo[MAXTASKS];

/*Question about task1:
1) Bisogna disattivare interrupt UART all'interno del for? (per evitare di sovrascrivere elementi dell'array mentre lì sto leggendo?);
 *  2) Se abbiamo sempre attivi gli interrupt anche negli altri task, come dovrebbero essere gestiti i messaggi e il buffer?;
 *  3) Si può fare senza interrup (leggendo direttamente da UxRDA);
 * */

void task1(parser_state* ps, CircularBuffer *cb,int* n1, int* n2, int* max, int* min, bool *hlena){
    int check;
    char value;
    char c = ',';
    int j = 0;
    int avl = avl_in_buffer(&cb);
    for(int i= 0; i < avl; i++){
        read_buffer(&cb, &value);
        char byte = *value;
        check = parse_byte(&ps, byte);
        char numero[10];
        char numero1[10];
        if(check == NEW_MESSAGE){
            if(strncmp(ps->msg_type,"HLREF",5) == 0){
                for(int j = 0; ps->msg_payload[j]!= c; j++){
                    numero[j] = ps->msg_payload[j];
                }
                *n1 = atoi(numero);
                for(int z = 0; ps->msg_payload[j+z]!= '\0'; z++){
                    numero1[z] = ps->msg_payload[j];
                }
                *n2 = atoi(numero1);
            }//if hlref
            else if(strncmp(ps->msg_type,"HLSAT",5) == 0){
               for(int j = 0; ps->msg_payload[j]!= c; j++){
                    numero[j] = ps->msg_payload[j];
                }
                *min = atoi(numero);
                for(int z = 0; ps->msg_payload[j+z]!= '\0'; z++){
                    numero1[z] = ps->msg_payload[j];
                }
                *max = atoi(numero1);
            }//if hlsat
            else if(strncmp(ps->msg_type,"HLENA",5) == 0){
                *hlena = *!hlena;
            }//if hlena 
        }//if check 
    } //for i
} //task 1() 

void scheduler() {//allows the execution of multiple tasks concurrently
    int i;
    for (i = 0; i < MAXTASKS; i++) {
        schedInfo[i].n++;//increment 'n' of all the struct in the schedInfo array
        if (schedInfo[i].n == schedInfo[i].N) {//when a 'n' of one task is equal to its N, the task is executed
            switch (i) {
                case 0:
                    task1();
                    break;
                case 1:
                    task2();
                    break;
                case 2:
                    task3();
                    break;
            }//switch(i)
            schedInfo[i].n = 0;//clear the n when n=N
        }//if schedInfo
    }//for i
} // scheduler

int main(void) {
    // parser initialization
    parse_init(&pstate);
    //TRISBbits.TRISB0 = 0; LED D3 output
    //TRISBbits.TRISB1 = 0; LED D4 output
    //TRISEbits.TRISE8 = 1; enable S5 button as input
    //TRISDbits.TRISD0 = 1; enable S6 button as input
    PWM_init();
    init_spi();
    init_uart();
    timer_setup_period();
    
    row_string first_row;
    row_string second_row;
    
    tmr_wait_ms(TIMER2,1000);
    int bigN[]={1,50,100}; //expire time
    int i;
    for(i=0;i<MAXTASKS;i++){
        schedInfo[i].N=bigN[i]; //set the expire time N for all task;
        schedInfo[i].n = 0;
    }
    // main loop
    while (1) {
        
        scheduler();
        // pdc(int &pdc1, int &pdc2, float t_pwm, int n1, int n2);
        PDC1 = *pdc1;
        PDC2 = *pdc2;
    }
    
    return 0;
}

