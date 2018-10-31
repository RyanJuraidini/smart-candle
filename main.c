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

/**
 * @param red_pwm = TA0CCR1 trigger value
 * @param green_pwm = TA1CCR1 trigger value
 * @return nothing
 */
void flameColor(int red_pwm, int green_pwm);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// set up port 1
	P1DIR |= BIT2;
	P1SEL |= BIT2;  // red 1.2 pwm
	// set up port 2
	P2DIR |= BIT1 + BIT4;
	P2SEL |= BIT1;  // green 2.1 pwm
	P2OUT &= ~BIT4;  // blue 2.4 off

	TA0CCR0 = 1000;
	TA0CCTL1 = OUTMOD_7;    // OUTMOD_7: reset/set (output set low when CCRx reached, high when CCR0 reached)
	TA0CCR1 = 1000;
	TA0CTL = TASSEL_2 + MC_1;   // TASSEL_2: use SMCLK (1MHz)
	                            // MC_1: up mode (count until CCR0)

	MC_0 + MC_1 + MC_2 + MC_3;

	TA1CCR0 = 1000;
	TA1CCTL1 = OUTMOD_7;
	//TA1CCTL2 = OUTMOD_3;
	TA1CCR1 = 1000;
	//TA1CCR2 = 1000;
	TA1CTL = TASSEL_2 + MC_1;

	// read analog values
	// map read values to pwm
	// write pwm to LEDs

	while(1)
	{

	    float power = (rand() % 200) + 800;     // generate random number from [0,20], shift to [80,100]
	    int red_pwm, green_pwm;
	    power = power / 1000;           // convert power to percent

	    // TODO: vary pwm based on random power level

	    red_pwm = 50;
	    green_pwm = 900;

	    flameColor(red_pwm, green_pwm);

	    __delay_cycles(1000);
	}

	return 0;
}

void flameColor(int red_pwm, int green_pwm)
{
    TA0CCR1 = red_pwm;
    TA1CCR1 = green_pwm;
}
