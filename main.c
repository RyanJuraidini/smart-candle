// M. Kaan Tasbas | mktasbas@gmail.com
// 10 October 2018

/* Pin descriptions:
 *  PWM:
 *      Timer A0 CCR1: P1.2 or P1.6 red
 *      Timer A1 CCR1: P2.1 or P2.2 green
 *      Timer A1 CCR2: P2.4 or P2.5
 *  Analog input:
 *      P1.0 - P1.7
 */

#include <msp430.h> 
#include <stdlib.h>

#define BUILT_IN_LED2 BIT6  // P1.6

// RGB LED pins
#define RED_LED     BIT2    // P1.2
#define GREEN_LED   BIT1    // P2.1
#define BLUE_LED    BIT4    // P2.4

// TILT SENSOR
#define TILT    BIT4        // P1.4

// increase MAX_RAND and LED_ON_FACTOR to make LED flicker slower
#define MAX_RAND 20
#define LED_ON_FACTOR 18

#define ADC_BUSY        ADC10CTL1 & BUSY
#define ADC_ARRAY_SIZE  128

// pwm = 0 on 100%, 100 on 0%
// flame color: red 5, green 90, blue 100

/**
 * @summary adjusts pwm wave of red and green pins of RGB
 * @param red_percent = red led duty cycle
 * @param green_percent = green led duty cycle
 * @return nothing
 */
void flameColor(int red_percent, int green_percent);

/**
 * @summary initializes pwm pins and timer modules used to generate waves
 * @param none
 * @return none
 */
void pwmInit(void);

void adcInit(void);
int readA0(void);

volatile unsigned long long cycles = 0;

int main(void)
{
    int red_duty = 95, green_duty = 5;
    int rand_on;
    unsigned long long tilt_cycle_count = 0;
    unsigned int adc_value = 0;
    unsigned int adc_history[ADC_ARRAY_SIZE] = {0}, adc_count = 0, adc_average = 0;
    unsigned long adc_sum = 0;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= BUILT_IN_LED2;

	//adcInit();
	pwmInit();


	while(1)
	{
	    rand_on = rand() % MAX_RAND;  // increase mod value here to make flicker less

	    // count number of cycles tilt sensor is active for
	    // helps avoid turning off LED when sensor is activated on drop
	    if(P1IN & TILT) tilt_cycle_count++;
	    else tilt_cycle_count = 0;

	    // turn off LED based on random value or tilt sensor
	    if( (rand_on > LED_ON_FACTOR) || (tilt_cycle_count > 100) ) flameColor(0, 0);
	    else flameColor(red_duty, green_duty);


/* ANALOG CODE
	    adc_value = (unsigned)readA0();
	    adc_sum += adc_value;

	    if(adc_count >= ADC_ARRAY_SIZE)
	    {
	        adc_count = 0;
	        adc_average = adc_sum / ADC_ARRAY_SIZE;
	        if(adc_average > 350)
	        {
	            P1OUT ^= BUILT_IN_LED2;
	        }
	        adc_sum = 0;
	    }
	    adc_history[adc_count++] = adc_value;
*/

	    //P1OUT ^= BUILT_IN_LED2;     // heart beat

	    cycles++;
	}

	return 0;
}

void pwmInit(void)
{
    // set up port 1
    P1DIR |= RED_LED;
    P1SEL |= RED_LED;  // red 1.2 pwm
    // set up port 2
    P2DIR |= GREEN_LED + BLUE_LED;
    P2SEL |= GREEN_LED;  // green 2.1 pwm
    P2OUT &= ~BLUE_LED;  // blue 2.4 off

    TA0CCR0 = 1000;
    TA0CCTL1 = OUTMOD_7;    // OUTMOD_7: reset/set (output set low when CCRx reached, high when CCR0 reached)
    TA0CCR1 = 1000;
    TA0CTL = TASSEL_2 + MC_1;   // TASSEL_2: use SMCLK (1MHz)
                                // MC_1: up mode (count until CCR0)

    TA1CCR0 = 1000;
    TA1CCTL1 = OUTMOD_7;
    //TA1CCTL2 = OUTMOD_3;
    TA1CCR1 = 1000;
    //TA1CCR2 = 1000;
    TA1CTL = TASSEL_2 + MC_1;

}

void flameColor(int red_percent, int green_percent)
{
    TA0CCR1 = red_percent * 10;     // red_percent/100 * 1000
    TA1CCR1 = green_percent * 10;   // green_percent/100 * 1000
}

void adcInit(void)
{
    ADC10CTL0 = ADC10SHT_2 | ADC10ON;
    ADC10CTL1 = 0x0000;
    while(ADC_BUSY);
    ADC10AE0 |= BIT0;
}

int readA0(void)
{
    ADC10CTL0 &= ~ENC;
    while(ADC_BUSY);
    ADC10CTL0 |= (ENC | ADC10SC);
    while(ADC_BUSY);
    return ADC10MEM;
}
