/* 
 * File:   glo.h
 * Author: franc
 *
 * Created on 21 September 2020, 17:38
 * Here We store the global variables that are used in more than one files in the project
 */

#ifndef GLO_H
#define	GLO_H

#include "xc.h"
#include "spi.h"
#include "pwm.h"
#include "parser.h"
#include "timer.h"
#include "uart.h"
#include <stdbool.h>
extern parser_state pstate;

extern long int Fosc;
extern long int Fcy; // = Fosc/4
extern float f_pwm; // 1000 Hz    
extern float t_pwm; // 1/f_pwm

extern volatile CircularBuffer cb;

extern int status;
extern bool print;
extern int count_print;

extern long int pdc2;
extern long int pdc1;
extern int n1;
extern int n2;

#endif	/* GLO_H */
