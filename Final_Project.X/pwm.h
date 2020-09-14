/* 
 * File:   pwm.h
 * Author: 
 *
 * Created on 08 September 2020, 16:22
 */

#ifndef PWM_H
#define	PWM_H

#define MAX_RPM 10000

float t_pwm;
void PWM_init();
void pdc(int *pdc1, int *pdc2, float t_pwm, int n1, int n2);
#endif	/* PWM_H */