#include "uart.h"
#include "timer.h"
#include <xc.h>
#include "glo.h"
#include"spi.h"

void init_uart(){
cb.writeIndex = 0;
cb.readIndex = 0;
U2BRG = 11; // (7372800 / 4) / (16 * 9600) - 1
U2MODEbits.UARTEN = 1; // enable UART
U2STAbits.UTXEN = 1; // enable U1TX
IEC1bits.U2RXIE = 1;
tmr_wait_ms(TIMER2, 1000);
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
char val = U2RXREG;
write_buffer(&cb, val);
}