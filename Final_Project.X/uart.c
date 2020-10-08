#include "uart.h"
#include "timer.h"
#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "glo.h"
#include "spi.h"

void init_uart(){
cb.writeIndex = 0;
cb.readIndex = 0;
U2BRG = 11; // (7372800 / 4) / (16 * 9600) - 1
U2MODEbits.UARTEN = 1; // enable UART
U2STAbits.UTXEN = 1; // enable U1TX
IEC1bits.U2RXIE = 1; // enable UART receiver interrupt 
}

void write_buffer( volatile CircularBuffer* cb, char value){
//parse_byte(&pstate, value);
//if( pstate->state == STATE_TYPE || pstate->state == STATE_PAYLOAD){
cb->buffer[cb->writeIndex] = value;
cb->writeIndex++;
if (cb->writeIndex == BUFFER_SIZE - 1)
    cb->writeIndex = 0;
}

int read_buffer( volatile CircularBuffer* cb, char* value){
IEC1bits.U2RXIE = 0;
if (cb->readIndex == cb->writeIndex){
    IEC1bits.U2RXIE = 1;
    return 0;
}
*value = cb->buffer[cb->readIndex];
cb->readIndex++;
if (cb->readIndex == BUFFER_SIZE)
    cb->readIndex = 0;
IEC1bits.U2RXIE = 1;
return 1;
}

int avl_in_buffer(volatile CircularBuffer* cb){
IEC1bits.U2RXIE = 0;
int wri = cb->writeIndex;
int rdi = cb->readIndex;
IEC1bits.U2RXIE = 1;
if (wri >= rdi){
    return wri - rdi;
    }
else{
    return wri - rdi + BUFFER_SIZE;
    }
}

void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _U2RXInterrupt(){
IFS1bits.U2RXIF = 0;
char val;
if (U2STAbits.OERR == 1){
// overflow occurred...
    int i = 0;
    for ( i = 0; i < 5; i++){ // empty buffer (4 values + U2RSR)
    val = U2RXREG;
    write_buffer(&cb, val);    
    }
U2STAbits.OERR = 0; // clear flag
}
while(U2STAbits.URXDA == 1){
    val = U2RXREG;
    write_buffer(&cb, val);
    }
}

void send_to_UART(char* str, int dim){
    char msg[dim];
    int i;
    
    sprintf(msg, "$%s*", str); 
    for(i = 0; msg[i] !='\0'; i++){
        if (U2STAbits.UTXBF == 0) { // Transmission Buffer not full
            U2TXREG = msg[i];
        }
        while (U2STAbits.UTXBF == 1) {
        } // Transmission Buffer Full, wait for space
        U2STAbits.OERR = 0; // Reset buffer overrun error
    }
}