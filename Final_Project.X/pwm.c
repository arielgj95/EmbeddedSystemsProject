#include"pwm.h"

void PWM_init(){
    
    /*In the Free Running mode, the PWM time base counts
    upwards until the value in the Time Base Period register
    (PTPER) is matched.*/
    PTCONbits.PTMOD = 0;
     
    // Setup the PWM2H pin
    PWMCON1bits.PEN1H = 1;

    PWMCON1bits.PEN2H = 1;
    
    float f_pwm = 1000; // 1000 Hz    
    float t_pwm = 1 / f_pwm;

    // PTPER must fit in 15 bits -> <32767
    // PTPER = Fcy/(f_pwm * PTMR_Prescaler) -1
    // With prescaler 1 PTER = 1842 -> Set prescaler at 1
    PTCONbits.PTCKPS = 0; // prescaler at 1:1
    // Could develop a function which return the prescaler value for the multiplication given the value of PTCKPS
    int PTMR_Prescaler = 1;

    PTPER = Fcy / (f_pwm * PTMR_Prescaler) - 1; //1842
    
    PTCONbits.PTEN = 1; // Turns on the PWM time base module
}

void pdc(int* pdc1, int* pdc2, float t_pwm, int n1, int n2){
    float temp1 = ((n1 + MAX_RPM)*t_pwm)/2*MAX_RPM;
    float temp2 = ((n2 + MAX_RPM)*t_pwm)/2*MAX_RPM;
    *pdc1 = temp1*2*PTPER/t_pwm;
    *pdc2 = temp2*2*PTPER/t_pwm;
}