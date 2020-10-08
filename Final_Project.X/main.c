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

#define MAXTASKS 6

typedef struct {//one struct for one task to be scheduled
    int n;//counter
    int N;//when n=N, the task have to be executed 
} heartbeat;


//GLOBAL VARIABLES
heartbeat schedInfo[MAXTASKS];
long int pdc1 = 0; //duty cycle of PN1H
long int pdc2 = 0; //duty cycle of PN2H
int n1 = 0; //motor reference 1
int n2 = 0; //motor reference 2

int min = MIN_RPM; //current minimum allowed value
int max = MAX_RPM; //current maximum allowed value
int min_old = MIN_RPM; //store minimum value to check if it has been changed
int max_old = MAX_RPM; //store maximum value to check if it has been changed

//variable used to print on LCD
char row[17];
char long_row[24];

float sum_temp = 0; //compute the average of the temperature 
int status = 0; // current status of the microcontroller 

//counters
int count_time = 0; //to check if enter in timeout mode
int temp_counter = 0; //temperature counter
int count_print = 0; //to manage long string LCD output

bool print = false; //flag to check what display in the LCD screen 

//pwm.h
long int Fosc = 7372800; 
long int Fcy = 1843200; // = Fosc/4
float f_pwm = 1000; //PWM frequency 
float t_pwm = 0.001; //PWM period
//parser.h
parser_state pstate;
//uart.h
volatile CircularBuffer cb;

void timeout(){ //Timeout mod. Stop the motors and set status = 1
    if(status != 2){
        n1 = 0;
        n2 = 0;
        pdc(&pdc1, &pdc2, t_pwm, n1, n2); //compute the desired duty cycles
        PDC1 = pdc1;
        PDC2 = pdc2;
        status = 1;//enter in time_out mode
    }
}

void task1(parser_state *ps, volatile CircularBuffer *cb,int *n1, int *n2, int *max, int *min){
    count_time++;
    if(count_time == 50){ //when counter = 50, last references was 5 seconds ago
        timeout();
    }
    int check;
    char value;
    char c = ',';
    int j , i, z, avl;
    i = j = z = avl = 0;

    avl = avl_in_buffer(cb); //count how many characters we hate to read
    
    for( i= 0; i < avl && avl > 0; i++){
        read_buffer(cb, &value);//store the received character in "value"
        check = parse_byte(ps,value);//check the returned value of parse_byte (see in parser.c))
        if(check == NEW_MESSAGE){//a new correct reference is received 
            count_time = 0;//restart counter for timeout mode
            if(status!= 2){//Not in safe mode
                status = 0; //controlled mod 
                char numero[7];
                char numero1[7];
                if(strncmp(ps->msg_type,"HLREF",5) == 0){ //HLREF message type received 
                    for(j = 0; ps->msg_payload[j]!= c; j++){
                        numero[j] = ps->msg_payload[j]; //read the number as string
                    }
                    *n1 = atoi(numero); //convert the string into number
                    for(z = j + 1; ps->msg_payload[z] != '\0'; z++){
                        numero1[z - j - 1] = ps->msg_payload[z];
                    }
                    *n2 = atoi(numero1);

                }//if hlref
                else if(strncmp(ps->msg_type,"HLSAT",5) == 0){ //HLSAT message type received 
                   for(j = 0; ps->msg_payload[j]!= c; j++){
                        numero[j] = ps->msg_payload[j];
                    }
                    *min = atoi(numero);
                    for( z = j + 1; ps->msg_payload[z] != '\0'; z++){
                        numero1[z - j - 1] = ps->msg_payload[z];
                    }
                    *max = atoi(numero1);
                }//if hlsat
            }//if status != 2
            //check HLENA only if the safe mode is active
            else if(strncmp(ps->msg_type,"HLENA",5) == 0){ //HLENA message type received 
                status = 0;//exit from safe mode
                send_to_UART("MCACK,ENA,1",(int)strlen("MCACK,ENA,1")); //send positive ack to UART
            }//if hlena     
        }//if check 
    } //for i
} //task 1()

void task2(int *n1, int *n2, int *min, int *max){
    if(*min <= 0 && *min >= MIN_RPM && *max >= 0 && *max <= MAX_RPM && *min < *max) //check if desired min and max values are allowed 
    {
    if(min_old != *min || max_old != *max){//set the new values if they are different 
        min_old = *min;
        max_old = *max;
        send_to_UART("MCACK,SAT,1",(int)strlen("MCACK,SAT,1")); //send positive ack to UART
        }
    }//if *min <= 0 && *min >= MIN_RPM && *max >= 0 && *max <= MAX_RPM && *min < *max
    else{ //wrong values! Keep the old ones
        *min = min_old;
        *max = max_old;
        send_to_UART("MCACK,SAT,0",(int)strlen("MCACK,SAT,0")); //send negative ack to UART
    }//else
    //Refresh the PWM to comply with the new saturation values
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
    //"print" flag changes when S6 is pressed
    if(!print){
        clear_row_LCD(SECOND_ROW); //clear and write the string in LCD second row
        sprintf(row,"RPM: %d,%d",*n1,*n2); //print RPM values
        write_string_LCD(row,(int)strlen(row));
    }//if !print
    if(print){
        clear_row_LCD(SECOND_ROW);
        sprintf(row,"RPM, %ld,%ld", pdc1, pdc2); //print duty cycle values
        write_string_LCD(row,(int)strlen(row));
    }//if print
}//task2

void task3(){
    sum_temp += acquire_temperature(); // acquire temperature from sensor
    temp_counter++; //increase the temperature counter 
    if(temp_counter == 10){ //when counter = 10
        sum_temp/=temp_counter;//compute the mean
        sprintf(row,"MCTEM,%.2f",sum_temp);
        send_to_UART(row,(int)strlen(row));//send ack to pc
        if(!print){//print on the first row of LCD temperature and status 
            clear_row_LCD(FIRST_ROW);
            if(status == 0){
            sprintf(row,"STA: C TEM: %.1f",sum_temp);
            write_string_LCD(row,(int)strlen(row));
            }//else status == 0
            else if(status == 1){
            sprintf(row,"STA: T TEM: %.1f",sum_temp);
            write_string_LCD(row,(int)strlen(row));
            }//else status == 1
            else if(status == 2){
            sprintf(row,"STA: H TEM: %.1f",sum_temp);
            write_string_LCD(row,(int)strlen(row));
            }//else status == 2
        }//if !print
    sum_temp = temp_counter = 0;//clear average and counter values
    }//if temp_counter    
}//task3

void task4(){ //blink LEDS  
    LATBbits.LATB0 = !LATBbits.LATB0; //blink always led D3
    if(status == 1)
        LATBbits.LATB1 = !LATBbits.LATB1; //blink led D4 in timeout status
    else
        LATBbits.LATB1 = 0;
}//task4

void task5(){ //send the periodic ack MCFBK to the pc 
    sprintf(long_row,"MCFBK,%d,%d,%d",n1,n2,status); 
    send_to_UART(long_row,(int)strlen(long_row)); //send ack every 200 ms
}//task5

void task6(){ //print the saturation values on LCD 
    if(print){
        clear_row_LCD(FIRST_ROW);
        sprintf(long_row,"SAT MIN:%d MAX:%d", min, max);
        write_long_string_LCD(long_row,count_print);//shift the string if it is too long
        if((int)strlen(long_row) - count_print <= 16){
            count_print = 0;
        }//if 
        else{
            count_print++;
        }//else
    }//if print
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
                    task4();
                    break;
                case 4:
                    task5();
                    break;
                case 5:
                    task6();
                    break;
            }//switch(i)
            schedInfo[i].n = 0;//clear the n when n=N
        }//if schedInfo
    }//for i
} // scheduler

int main(void) { 
    parse_init(); // parser initialization
    PWM_init(); //PWM initialization
    init_spi(); //SPI initialization
    init_uart(); //UART initialization
    init_adc(); // ADC initialization
    init_leds(); //leds initialization
    init_buttons();//buttons initialization

    tmr_setup_period(TIMER1,100); //setup and start TIMER 1, timer of the scheduler
    
    tmr_wait_ms(TIMER3,1000); //wait one second at startup
    
    //scheduler
    int bigN[]={1,1,1,5,2,5}; //expire time
    int i;
    for(i=0;i<MAXTASKS;i++){
        schedInfo[i].N=bigN[i]; //set the expire time N for all task;
        schedInfo[i].n = 0;
    }

    // main loop
    while (1){
        scheduler();
        tmr_wait_period(TIMER1); // wait what is needed for the next loop
        }
    return 0;
}