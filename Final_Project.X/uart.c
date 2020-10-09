#include "uart.h"
#include "timer.h"
#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "glo.h"
#include "spi.h"

void init_uart(){
cb.writeIndex = 0; //initialize the write and read index of the circular buffer
cb.readIndex = 0;
U2BRG = 11; // (7372800 / 4) / (16 * 9600) - 1 , choose 9600 as baud rate
U2MODEbits.UARTEN = 1; // enable UART
U2STAbits.UTXEN = 1; // enable U1TX
IEC1bits.U2RXIE = 1; // enable UART receiver interrupt 
}

void write_buffer( volatile CircularBuffer* cb, char value){
cb->buffer[cb->writeIndex] = value; //go to the buffer of Circular Buffer struct and put the value in the write index position
cb->writeIndex++; //increment the write_index 
if (cb->writeIndex == BUFFER_SIZE - 1) //If I reached the last element of the array, put the write_index to 0,
    cb->writeIndex = 0;                // doing this, I restart to write again on the buffer from its initial position (this is why it is "circular")
}

int read_buffer( volatile CircularBuffer* cb, char* value){
IEC1bits.U2RXIE = 0;        //disable the uart interrupts (otherwise read and write index can be updated while I'm trying to read them)
if (cb->readIndex == cb->writeIndex){ //If I finished to read, reactivate the interrupts and return 0
    IEC1bits.U2RXIE = 1;
    return 0;
}
*value = cb->buffer[cb->readIndex]; //take the value inside the circular buffer in the position "read_index"
cb->readIndex++; //increment the read_index
if (cb->readIndex == BUFFER_SIZE) //If I reached the end of the buffer, put read_index=0 to read from the initial positon (this is why it is "circular")
    cb->readIndex = 0;
IEC1bits.U2RXIE = 1; //activate the interrupts
return 1;
}

int avl_in_buffer(volatile CircularBuffer* cb){
IEC1bits.U2RXIE = 0; //disable the interrupts (for the same reason as read_buffer function)
int wri = cb->writeIndex; //take the write and read indices
int rdi = cb->readIndex;
IEC1bits.U2RXIE = 1; //reactivate the interrupts
if (wri >= rdi){  //if the write index is >= read_index, then return the number of characters that I have to read
    return wri - rdi;
    }
else{
    return wri - rdi + BUFFER_SIZE; //otherwise do the same but adding the size of the buffer (read until I reach the end of the buffer and then restart again  
    }                               // from the first position of the array until I reach the position of the write_index)
}

void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _U2RXInterrupt(){ //ISR of the uart
IFS1bits.U2RXIF = 0;  //clear the UART interrupt flag to 0
char val;
if (U2STAbits.OERR == 1){ // if OERR=1, an overflow occurred...
    int i = 0;
    for ( i = 0; i < 5; i++){ // empty buffer (4 values + U2RSR)
    val = U2RXREG;   //take the values from the receiver buffer of the UART and put them to the buffer of the Circular Buffer struct
    write_buffer(&cb, val);    
    }
U2STAbits.OERR = 0; // clear the flag
}
while(U2STAbits.URXDA == 1){ //There are some values to read in the Receiver Buffer of the UART
    val = U2RXREG;  
    write_buffer(&cb, val);
    }
}

void send_to_UART(char* str, int dim){
    char msg[dim];
    int i;
    
    sprintf(msg, "$%s*", str);   //add the $ symbol before the string and the * to the end of the string to create a message to be sent
    for(i = 0; msg[i] !='\0'; i++){
        if (U2STAbits.UTXBF == 0) { // Transmission Buffer not full
            U2TXREG = msg[i]; //put a character in the transmit register of the UART
        }
        while (U2STAbits.UTXBF == 1) { 
        } // Transmission Buffer Full, wait for some empty space inside it
        U2STAbits.OERR = 0; // Reset buffer overrun error
    }
}
