#include"pwm.h"
#include<xc.h>
#include "glo.h"

void PWM_init(){
    
    /*In the Free Running mode, the PWM time base counts
    upwards until the value in the Time Base Period register
    (PTPER) is matched.*/
    PTCONbits.PTMOD = 0;
    PWMCON1bits.PEN1H = 1; // Setup the PWM1H pin
    PWMCON1bits.PEN2H = 1;// Setup the PWM2H pin

    // PTPER must fit in 15 bits -> <32767
    // PTPER = Fcy/(f_pwm * PTMR_Prescaler) -1
    // With prescaler 1 PTER = 1842 -> Set prescaler at 1
    PTCONbits.PTCKPS = 0; // prescaler at 1:1
    int PTMR_Prescaler = 1; 
    PTPER = Fcy / (f_pwm * PTMR_Prescaler) - 1; //1842
    PTCONbits.PTEN = 1; // Turns on the PWM time base module
}

void pdc(long int* pdc1, long int* pdc2, float t_pwm, int n1, int n2){
    int real_ptper = PTPER; //store the time period in a variable
    //These formulas derive from the fact that the RPM can have 20000 different values (from -10000 to 10000)
    //while duty cycles can have 2*1842 different values -> (PDC = Tcy/2))
    float temp1 = ((n1 + MAX_RPM2)*t_pwm)/(2*MAX_RPM2); 
    float temp2 = ((n2 + MAX_RPM2)*t_pwm)/(2*MAX_RPM2); 
    *pdc1 = temp1*2*real_ptper/t_pwm;//compute the new PDC1 value
    *pdc2 = temp2*2*real_ptper/t_pwm;//compute the new PDC2 value 
}