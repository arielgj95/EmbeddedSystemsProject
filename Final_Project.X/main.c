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
#include "glo.h"
#include "adc.h"

#define MAXTASKS 3

typedef struct {//one struct for one task to be scheduled
    int n;//counter
    int N;//when n=N, the task have to be executed 
} heartbeat;


//GLOBAL VARIABLES
int pdc1 = 0;
int pdc2 = 0;
heartbeat schedInfo[MAXTASKS];
int temp_counter = 0;
int n1 = 0;
int n2 = 0;
int min = MIN_RPM;
int max = MAX_RPM;
bool hlena = false;
char max1[10];
char min1[10];
char num1[10];
char num2[10];

float temp_buffer[10];
float sum_temp = 0;

//pwm.h
long int Fosc = 7372800;
long int Fcy = 1843200;
float f_pwm = 1000;
float t_pwm = 0.001;
//parser.h
parser_state pstate;
//uart.h
volatile CircularBuffer cb;
//spi.h
row_string first_row;
row_string second_row;

/*Question about task1:
1) Bisogna disattivare interrupt UART all'interno del for? (per evitare di sovrascrivere elementi dell'array mentre lì sto leggendo?);
 *  2) Se abbiamo sempre attivi gli interrupt anche negli altri task, come dovrebbero essere gestiti i messaggi e il buffer?;
 *  3) Si può fare senza interrup (leggendo direttamente da UxRDA);
 * */

void task1(parser_state *ps, volatile CircularBuffer *cb,int *n1, int *n2, int *max, int *min, bool *hlena){
    int check;
    char value;
    char c = ',';
    int j , i, z, read, avl;
    i = j = z = avl = 0;

    avl = avl_in_buffer(cb); 
    
    for( i= 0; i < avl && avl > 0; i++){
        read = read_buffer(cb, &value);
        check = parse_byte(ps,value);
        if(check == NEW_MESSAGE){
            //write_string_LCD("yes",10);
            char numero[10];
            char numero1[10];
            //char string[20];
            if(strncmp(ps->msg_type,"HLREF",5) == 0){
                for(j = 0; ps->msg_payload[j]!= c; j++){
                    numero[j] = ps->msg_payload[j];
                }
                *n1 = atoi(numero);
                for(z = j + 1; ps->msg_payload[z] != '\0'; z++){
                    numero1[z - j - 1] = ps->msg_payload[z];
                }
                *n2 = atoi(numero1);
                //strcat(numero,numero1);
            }//if hlref
            else if(strncmp(ps->msg_type,"HLSAT",5) == 0){
               for(j = 0; ps->msg_payload[j]!= c; j++){
                    numero[j] = ps->msg_payload[j];
                }
                *min = atoi(numero);
                for( z = j + 1; ps->msg_payload[z] != '\0'; z++){
                    numero1[z - j - 1] = ps->msg_payload[z];
                }
                *max = atoi(numero1);
                //strcat(numero,numero1);
            }//if hlsat
            else if(strncmp(ps->msg_type,"HLENA",5) == 0){
                *hlena = !*hlena;
            }//if hlena     
        }//if check 
    } //for i
} //task 1() 

void task2(int *n1, int *n2, int *min, int *max){
    if(*min <= 0 && *min >= MIN_RPM && *max >= 0 && *max <= MAX_RPM && min < max)
    {
        pdc(&pdc1, &pdc2, t_pwm, n1, n2);
        PDC1 = *pdc1;
        PDC2 = *pdc2;
    }
}

void task3(){
    temp_buffer[temp_counter] = acquire_temperature();
    sum_temp += temp_buffer[temp_counter];
    temp_counter++;
    if(temp_counter == 9){
        sum_temp/=temp_counter;
        //TO DO --> send ack to pc
        sum_temp = temp_counter = 0;
    }
}

void scheduler() {//allows the execution of multiple tasks concurrently
    int i;
    for (i = 0; i < MAXTASKS; i++) {
        schedInfo[i].n++;//increment 'n' of all the struct in the schedInfo array
        if (schedInfo[i].n == schedInfo[i].N) {//when a 'n' of one task is equal to its N, the task is executed
            switch (i) {
                case 0:
                    task1(&pstate, &cb, &n1,  &n2,  &max, &min, &hlena);
                    break;
                case 1:
                    task2(&n1, &n2, &min, &max);
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
    //TRISBbits.TRISB0 = 0; LED D3 output
    //TRISBbits.TRISB1 = 0; LED D4 output
    //TRISEbits.TRISE8 = 1; enable S5 button as input
    //TRISDbits.TRISD0 = 1; enable S6 button as input
    
    // parser initialization
    parse_init(&pstate);
    PWM_init();
    init_spi();
    init_uart();
    adc_init();
    
    tmr_setup_period(TIMER1,100);
    
    tmr_wait_ms(TIMER2,1000);
    /*
    int bigN[]={1,50,100}; //expire time
    int i;
    for(i=0;i<MAXTASKS;i++){
        schedInfo[i].N=bigN[i]; //set the expire time N for all task;
        schedInfo[i].n = 0;
    }
    */
    // main loop
    while (1){
        scheduler();
        /*
            sprintf(max1,"%d",max);
            write_string_LCD(max1,10);
        }
        */
        tmr_wait_period(TIMER1);
        }
    return 0;
}