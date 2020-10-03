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
clear_row(&first_row);
clear_row(&second_row);
}

void clear_row(row_string* row){
int i = 0;
for ( i = 0; i < 16; i++){
    row->string[i] = ' ';
}
row->size = 0;
}

void shift_string (char* str , int n, int shiftsize ){
int i ;
for ( i = 0; i < (n - shiftsize ); i++){
    str[i] = str[i + shiftsize];
}
}

void write_string_LCD(char* str, int max){
int i = 0;
for(i = 0;str[i]!= '\0' && i < max; i++){
    while (SPI1STATbits.SPITBF == 1); // wait for previous transmissions to finish
    SPI1BUF = str[i];
    }
}

void write_long_string_LCD(char* str, int counter){
int i;
for(i = counter; str[i]!= '\0' && i < counter+16; i++){
    while (SPI1STATbits.SPITBF == 1); // wait for previous transmissions to finish
    SPI1BUF = str[i];
    }
}

void move_cursor_first_row(){
while (SPI1STATbits.SPITBF == 1);
SPI1BUF = 0x80;
//tmr_wait_ms(TIMER3, 250);
}
   
void move_cursor_second_row(){
while (SPI1STATbits.SPITBF == 1);
SPI1BUF = 0xC0;
//tmr_wait_ms(TIMER3, 250);
}

void clear_row_LCD(int flag){
    if(!flag){
        move_cursor_first_row();
        write_string_LCD("                ",16);
        move_cursor_first_row();
    }
    else{
        move_cursor_second_row();
        write_string_LCD("                ",16);
        move_cursor_second_row();       
    }
}