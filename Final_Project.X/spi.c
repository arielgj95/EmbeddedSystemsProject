#include "spi.h"
#include "timer.h"
#include"xc.h"
#include "glo.h"
#include <stdbool.h>

void init_spi(){
SPI1CONbits.MSTEN = 1; // master mode
SPI1CONbits.MODE16 = 0; // 8?bit mode
SPI1CONbits.PPRE = 3; // 1:1 primary prescaler
SPI1CONbits.SPRE = 6; // 2:1 secondary prescaler
SPI1STATbits.SPIEN = 1; // enable SPI
}

void shift_string (char* str , int n, int shiftsize ){ //la usiamo?
int i ;
for ( i = 0; i < (n - shiftsize ); i++){
    str[i] = str[i + shiftsize];
}
}

void write_string_LCD(char* str, int max){
int i = 0;
for(i = 0;str[i]!= '\0' && i < max; i++){ //for all the carachters inside the string 
    while (SPI1STATbits.SPITBF == 1); // wait for previous transmissions to finish
    SPI1BUF = str[i]; //put the carachter into the lcd
    }
}

void write_long_string_LCD(char* str, int counter){
int i;
for(i = counter; str[i]!= '\0' && i < counter+16; i++){ //starting from the position counter
    while (SPI1STATbits.SPITBF == 1); // wait for previous transmissions to finish
    SPI1BUF = str[i];   //put into the lcd the charchter in that position and the following 16 ones
    }
}

void move_cursor_first_row(){
while (SPI1STATbits.SPITBF == 1); //If 1,transmit not yet started because the Transmit Buffer is full, wait for previous transmission to finish
SPI1BUF = 0x80; //send to the SPI1 slave the address of the first element of the first row
}
   
void move_cursor_second_row(){
while (SPI1STATbits.SPITBF == 1);
SPI1BUF = 0xC0; //send to the SPI1 slave the address of the first element of the second row
}

void clear_row_LCD(int flag){ //depending on the flag:
    if(!flag){
        move_cursor_first_row(); //go to the first element of the first row
        write_string_LCD("                ",16); //write 16 "black space" characters
        move_cursor_first_row(); //then go again to the first element of the first row
    }
    else{       // do the same as before but for the second row instead of the first one
        move_cursor_second_row();
        write_string_LCD("                ",16);
        move_cursor_second_row();       
    }
}
