/* 
 * File:   spi.h
 * Author: franc
 *
 * Created on 09 September 2020, 16:30
 */

#ifndef SPI_H
#define	SPI_H

typedef struct {
char string[17];
int size;
} row_string;

void init_spi();
void clear_row(row_string* row);
void shift_string(char* str , int n, int shiftsize);
void write_string_LCD(char* str, int max);
void move_cursor_first_row();
void move_cursor_second_row();
#endif	/* SPI_H */

