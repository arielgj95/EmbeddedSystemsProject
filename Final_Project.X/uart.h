/* 
 * File:   uart.h
 * Author: franc
 *
 * Created on 09 September 2020, 16:30
 */

#ifndef UART_H
#define	UART_H

#define BUFFER_SIZE 60
typedef struct{
char buffer[BUFFER_SIZE];
int readIndex;
int writeIndex;
} CircularBuffer;

volatile CircularBuffer cb;

void init_uart();
void write_buffer( volatile CircularBuffer* cb, char value);
int read_buffer( volatile CircularBuffer* cb, char* value);
int avl_in_buffer( volatile CircularBuffer* cb);
#endif	/* UART_H */

