/* 
 * File:   glo.h
 * Author: franc
 *
 * Created on 21 September 2020, 17:38
 */

#ifndef GLO_H
#define	GLO_H

#include "xc.h"
#include "spi.h"
#include "pwm.h"
#include "parser.h"
#include "timer.h"
#include "uart.h"

extern parser_state pstate;

extern long int Fosc;
extern long int Fcy; // = Fosc/4
extern float f_pwm; // 1000 Hz    
extern float t_pwm; // 1/f_pwm

extern volatile CircularBuffer cb;

extern row_string first_row;
extern row_string second_row;

#endif	/* GLO_H */