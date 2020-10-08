/* 
 * File:   spi.h
 * Author: franc
 *
 * Created on 09 September 2020, 16:30
 */

#ifndef SPI_H
#define	SPI_H

#define FIRST_ROW (0)
#define SECOND_ROW (1)

void init_spi();
void clear_row_LCD(int flag);
void shift_string(char* str , int n, int shiftsize);
void write_string_LCD(char* str, int max);
void write_long_string_LCD(char* str, int counter);
void move_cursor_first_row();
void move_cursor_second_row();
#endif	/* SPI_H */