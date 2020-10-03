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
#include "buttonled1.h"

#define MAXTASKS 5

typedef struct {//one struct for one task to be scheduled
    int n;//counter
    int N;//when n=N, the task have to be executed 
} heartbeat;


//GLOBAL VARIABLES
heartbeat schedInfo[MAXTASKS];
long int pdc1 = 0;
long int pdc2 = 0;
int n1 = 0;
int n2 = 0;

int min = MIN_RPM;
int max = MAX_RPM;
int min_old = MIN_RPM;
int max_old = MAX_RPM;

char max1[10];
char min1[10];
char num1[10];
char num2[10];

float temp_buffer[10];
float sum_temp = 0;
int status = 0;

int count_time = 0;
int temp_counter = 0;
int count_print = 0;
bool print = false;

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

/*Questions about task1:
1) Bisogna disattivare interrupt UART all'interno del for? (per evitare di sovrascrivere elementi dell'array mentre lì sto leggendo?);
 *  2) Se abbiamo sempre attivi gli interrupt anche negli altri task, come dovrebbero essere gestiti i messaggi e il buffer?;
 *  3) Si può fare senza interrup (leggendo direttamente da UxRDA);
 *  4) Impostare di nuovo la velocità dei motori = a zero quando già l'ho fatto prima nell'interrupt?
 *  5) frequenza a cui lampeggia D4?
 *  6) Quale terminale abbiamo usato a lezione? Noi ne abbiamo preso uno a babbo su internet
 *  7) Dobbiamo stampare la temperatura corrente o la media degli ultimi 10 campioni?
 * */

void timeout(int *n1, int *n2){
    if(status != 2){
        *n1 = 0;
        *n2 = 0;
        pdc(&pdc1, &pdc2, t_pwm, *n1, *n2);
        PDC1 = pdc1;
        PDC2 = pdc2;
        status = 1;//enter in time_out mode

    }
}

void task1(parser_state *ps, volatile CircularBuffer *cb,int *n1, int *n2, int *max, int *min){
    count_time++;
    if(count_time == 50){
        timeout(n1, n2);
    }
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
            count_time = 0;
            if(status!= 2){
                status = 0; 
                char numero[10];
                char numero1[10];
                if(strncmp(ps->msg_type,"HLREF",5) == 0){
                    for(j = 0; ps->msg_payload[j]!= c; j++){
                        numero[j] = ps->msg_payload[j];
                    }
                    *n1 = atoi(numero);
                    for(z = j + 1; ps->msg_payload[z] != '\0'; z++){
                        numero1[z - j - 1] = ps->msg_payload[z];
                    }
                    *n2 = atoi(numero1);

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

                }//if hlsat
            }
            else if(strncmp(ps->msg_type,"HLENA",5) == 0){
                status = 0;
                send_to_UART("MCACK,ENA,1",(int)strlen("MCACK,ENA,1")); //send positive ack to UART
            }//if hlena     
        }//if check 
    } //for i
} //task 1() 

void task2(int *n1, int *n2, int *min, int *max){
    if(*min <= 0 && *min >= MIN_RPM && *max >= 0 && *max <= MAX_RPM && *min < *max)
    {
    if(min_old != *min || max_old != *max){
        min_old = *min;
        max_old = *max;
        send_to_UART("MCACK,SAT,1",(int)strlen("MCACK,SAT,1")); //send positive ack to UART
        }
    }
    else{
        *min = min_old;
        *max = max_old;
        send_to_UART("MCACK,SAT,0",(int)strlen("MCACK,SAT,0")); //send negative ack to UART
    }
    if(*min > *n1) 
        *n1 = *min;
    if(*max < *n1)
        *n1 = *max;
    if(*min > *n2) 
        *n2 = *min;
    if(*max < *n2)
        *n2 = *max;
    pdc(&pdc1, &pdc2, t_pwm, *n1, *n2);
    PDC1 = pdc1;
    PDC2 = pdc2;
    if(!print){
        clear_row_LCD(SECOND_ROW);
        sprintf(second_row.string,"RPM: %d,%d",*n1,*n2);
        write_string_LCD(second_row.string,(int)strlen(second_row.string));
    }
    if(print){
        clear_row_LCD(SECOND_ROW);
        sprintf(second_row.string,"RPM, %ld,%ld", pdc1, pdc2);
        write_string_LCD(second_row.string,(int)strlen(second_row.string));
    }
}

void task3(){
    char temp_avr[15];
    temp_buffer[temp_counter] = acquire_temperature();
    sum_temp += temp_buffer[temp_counter];
    temp_counter++;
    if(temp_counter == 10){
        sum_temp/=temp_counter;
        sprintf(temp_avr,"MCTEM,%f",sum_temp);
        send_to_UART(temp_avr,(int)strlen(temp_avr));//TO DO --> send ack to pc
        sum_temp = temp_counter = 0;
    }    
    if(!print){
        clear_row_LCD(FIRST_ROW);
        if(status == 0){
        sprintf(first_row.string,"STA: C TEM: %.1f",temp_buffer[temp_counter]);
        write_string_LCD(first_row.string,(int)strlen(first_row.string));
        }
        else if(status == 1){
        sprintf(first_row.string,"STA: T TEM: %.1f",temp_buffer[temp_counter]);
        write_string_LCD(first_row.string,(int)strlen(first_row.string));
        }
        else if(status == 2){
        sprintf(first_row.string,"STA: H TEM: %.1f",temp_buffer[temp_counter]);
        write_string_LCD(first_row.string,(int)strlen(first_row.string));
        }
    }
}

void task4(int *min, int *max){
    LATBbits.LATB0 = !LATBbits.LATB0; //blink led D3
    if(status == 1)
        LATBbits.LATB1 = !LATBbits.LATB1; //blink led D4 in time_out stauts
    else
        LATBbits.LATB1 = 0;
    if(print){
        clear_row_LCD(FIRST_ROW);
        char long_row[24];
        sprintf(long_row,"SAT MIN:%d MAX:%d", *min, *max);
        write_long_string_LCD(long_row,count_print);
        if((int)strlen(long_row) - count_print == 16){
            count_print = 0;
        }
        else{
            count_print++;
        }
    }
}

void task5(int *n1, int *n2){
    char str[20];
    sprintf(str,"MCFBK,%d,%d,%d",*n1,*n2,status);
    send_to_UART(str,(int)strlen(str));
}

void scheduler() {//allows the execution of multiple tasks concurrently
    int i;
    for (i = 0; i < MAXTASKS; i++) {
        schedInfo[i].n++;//increment 'n' of all the struct in the schedInfo array
        if (schedInfo[i].n == schedInfo[i].N) {//when a 'n' of one task is equal to its N, the task is executed
            switch (i) {
                case 0:
                    task1(&pstate, &cb, &n1,  &n2,  &max, &min);
                    break;
                case 1:
                    task2(&n1, &n2, &min, &max);
                    break;
                case 2:
                    task3();
                    break;
                case 3:
                    task4(&min, &max);
                    break;
                case 4:
                    task5(&n1, &n2);
            }//switch(i)
            schedInfo[i].n = 0;//clear the n when n=N
        }//if schedInfo
    }//for i
} // scheduler

int main(void) { 
    // parser initialization
    parse_init(&pstate);
    PWM_init();
    init_spi();
    init_uart();
    init_adc();
    init_leds();
    init_buttons();
   
    tmr_setup_period(TIMER1,100);
    
    tmr_wait_ms(TIMER3,1000);
    
    //scheduler
    int bigN[]={1,1,1,5,2}; //expire time
    int i;
    for(i=0;i<MAXTASKS;i++){
        schedInfo[i].N=bigN[i]; //set the expire time N for all task;
        schedInfo[i].n = 0;
    }
    
    // main loop
    while (1){
        scheduler();
        tmr_wait_period(TIMER1);
        }
    return 0;
}