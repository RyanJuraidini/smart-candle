// M. Kaan Tasbas | mktasbas@gmail.com
// 10 October 2018

/* Pin descriptions:
 *  PWM:
 *      Timer A0 CCR1: P1.2 or P1.6
 *      Timer A1 CCR1: P2.1 or P2.2
 *      Timer A1 CCR2: P2.4 or P2.5
 *  Analog input:
 *      P1.0 - P1.7
 *      P1.3 VREF-/VEREF-
 *      P1.4 VREF+/VEREF+
 */

// PWM sample code: https://www.kompulsa.com/example-code-msp430-pwm/

#include <msp430.h> 
#include <stdlib.h>

// RGB LED pins
#define RED_LED     BIT2    // P1.2
#define GREEN_LED   BIT1    // P2.1
#define BLUE_LED    BIT4    // P2.4

// analog read pins
#define V0          BIT0    // P1.0 -> A0
#define V1          BIT1    // P1.1 -> A1

// increase MAX_RAND and LED_ON_FACTOR to make LED flicker slower
#define MAX_RAND 20
#define LED_ON_FACTOR 18

void printf(char *, ...);

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

void analogInit(void);

int readA0(void);

int a0_reading = 0;

int main(void)
{
    int red_duty = 95, green_duty = 5;
    int rand_on;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__enable_interrupt();       // enable global interrupts
	

	pwmInit();
	analogInit();

	while(1)
	{
	    rand_on = rand() % MAX_RAND;  // increase mod value here to make flicker less

	    flameColor(red_duty, green_duty);

	    if(rand_on > LED_ON_FACTOR) flameColor(0, 0);

	    //a0_reading = readA0();

	    //printf("%i\r\n", a0_reading);
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

    // RED LED
    TA0CCR0 = 1000;
    TA0CCTL1 = OUTMOD_7;    // OUTMOD_7: reset/set (output set low when CCRx reached, high when CCR0 reached)
    TA0CCR1 = 1000;
    TA0CTL = TASSEL_2 + MC_1;   // TASSEL_2: use SMCLK (1MHz)
                                // MC_1: up mode (count until CCR0)

    // GREEN (CCR1), BLUE (CCR2) LED
    TA1CCR0 = 1000;
    TA1CCTL1 = OUTMOD_7;
    //TA1CCTL2 = OUTMOD_7;
    TA1CCR1 = 1000;
    TA1CCR2 = 1000;
    TA1CTL = TASSEL_2 + MC_1;

    TA1CCTL2 = CCIE;

}

void flameColor(int red_percent, int green_percent)
{
    TA0CCR1 = red_percent * 10;     // red_percent/100 * 1000
    TA1CCR1 = green_percent * 10;   // green_percent/100 * 1000

    TA1CCR2 = red_percent * 10;
}

void analogInit(void)
{
    /**
     * initialize ADC10CTL0 register
     * SREF_0 -> V+ = Vcc, V- = Vss
     * ADC10SHT_2 -> 16 x ADC10CLK sample and hold time
     * ~ADC10SR -> buffer supports ~200 ksps (200,000 samples/second)
     * ~REFOUT -> ref output off
     * ~REFBURST -> ref buffer always on
     * ~MSC -> single sample
     * ~REF2_5V -> not used
     * ~REFON -> reference generator off
     * ADC10ON -> ADC10 on
     * ~ADC10IE -> interrupt disabled
     * ADC10IFG -> set when ADC10MEM is loaded with result, automatically reset
     * ENC -> enable conversion
     * ADC10SC -> start conversion, automatically reset
     */
    ADC10CTL0 = ADC10SHT_2 | ADC10ON | ADC10IE;

    /**
     * initialize ADC10CTL1 register
     * INCH_0 -> select channel A0
     * SHS_0 -> sample and hold source select, ADC10SC bit
     * ~ADC10DF -> straight binary data format
     * ~ISSH -> sample-input signal not inverted
     * ADC10DIV_0 -> ADC10 clock divider, /1
     * ADC10SEL_0 -> ADC10 clock source, ADC10OSC
     * CONSEQ_0 -> single channel single conversion
     * ADC10BUSY -> flag, 0 if no operation, 1 if conversion active
     */
    ADC10CTL1 = INCH_0;

    // wait if ADC10 is busy
    while(ADC10CTL1 & BUSY);

    // single conversion, single data transfer
    //ADC10DTC1 = 0x01;

    // input enable
    ADC10AE0 |= (V1);

    //ADC10CTL0 |= ENC; // enable ADC and start conversion

}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    a0_reading = ADC10MEM;
    //a0_reading = a0_reading >> 6;
}

#pragma vector=TIMER1_A2_VECTOR
__interrupt void TIMER0_A2_ISR(void)
{
    ADC10CTL0 |= ADC10SC;
}

int readA0(void)
{
    int i = 0, input = 0;

    for(i = 0; i < 5; i++)
    {
        ADC10CTL0 &= ~ENC;          // enable conversion
        while(ADC10CTL1 & BUSY);    // wait if ADC10 is busy
        ADC10SA = (unsigned)&input; // RAM address to put reading
        ADC10CTL0 |= ENC | ADC10SC; // start conversion
        while(ADC10CTL1 & BUSY);    // wait for conversion to finish

        input += input;
    }

    input = input / 5; // average 5 readings from input

    return input;
}
