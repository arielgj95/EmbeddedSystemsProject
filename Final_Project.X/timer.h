/* 
 * File:   timer.h
 * Author: franc
 *
 * Created on 10 September 2020, 15:57
 */

#ifndef TIMER_H
#define	TIMER_H

#define TIMER1 (1)
#define TIMER2 (2)

void timer_setup_period(int n, int ms);
void timer_wait_period(int n);
void choose_prescaler(int ms, int* tckps, int* pr);
void tmr_wait_ms(int n, int ms);

#endif	/* TIMER_H */
