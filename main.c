// M. Kaan Tasbas | mktasbas@gmail.com
// 10 October 2018

#include <msp430.h> 

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// read analog values
	// map read values to pwm
	// write pwm to LEDs

	return 0;
}
