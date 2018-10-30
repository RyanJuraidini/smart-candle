// M. Kaan Tasbas | mktasbas@gmail.com
// 10 October 2018

/* Pin descriptions:
 *  PWM:
 *      Timer A0 CCR1: P1.2 or P1.6
 *      Timer A1 CCR1: P2.1 or P2.2
 *      Timer A1 CCR2: P2.4 or P2.5
 *  Analog input:
 *      P1.0 - P1.7
 */

// PWM sample code: https://www.kompulsa.com/example-code-msp430-pwm/

#include <msp430.h> 
#include <stdlib.h>

// pwm = 0 on 100%, 100 on 0%
// flame color: red 5, green 90, blue 100
void setRedPwm(int pwm);
void setGreenPwm(int pwm);
void setBluePwm(int pwm);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= BIT2;
	P1SEL |= BIT2;  // red 1.2
	P2DIR |= BIT1 + BIT4;
	P2SEL |= BIT1 + BIT4;   // green 2.1 + blue 2.4

	TA0CCR0 = 1000;
	TA0CCTL1 = OUTMOD_3;
	TA0CCR1 = 1000;
	TA0CTL = TASSEL_2 + MC_1;

	TA1CCR0 = 1000;
	TA1CCTL1 = OUTMOD_3;
	TA1CCTL2 = OUTMOD_3;
	TA1CCR1 = 1000;
	TA1CCR2 = 1000;
	TA1CTL = TASSEL_2 + MC_1;

	// read analog values
	// map read values to pwm
	// write pwm to LEDs

	while(1)
	{
//	    setRedPwm(0);
//	    setGreenPwm(100);
//	    setBluePwm(100);
//
//	    __delay_cycles(500000);
//
//	    setRedPwm(100);
//	    setGreenPwm(0);
//	    setBluePwm(100);
//
//	    __delay_cycles(500000);
//
//	    setRedPwm(100);
//	    setGreenPwm(100);
//	    setBluePwm(0);
//
//	    __delay_cycles(500000);

	    float power = (rand() % 200) + 800;     // generate random number from [0,20], shift to [80,100]
	    int red_pwm, green_pwm;
	    power = power / 1000;           // convert power to percent
//	    power = 1 - power;              // invert power because 0 is max power

	    // TODO: vary pwm based on random power level. Current calculation isn't working

	    red_pwm = 100 + (50 * power);                  // set pwm to 100 +- 50
	    green_pwm = 900 + (50 * power);                // set pwm to 900 +- 50



	    setRedPwm(red_pwm);
	    setGreenPwm(green_pwm);
	    setBluePwm(1000);

	    __delay_cycles(1000);
	}

	return 0;
}

void setRedPwm(int pwm)
{
    TA0CCR1 = pwm;
}

void setGreenPwm(int pwm)
{
    TA1CCR1 = pwm;
}

void setBluePwm(int pwm)
{
    TA1CCR2 = pwm;
}
