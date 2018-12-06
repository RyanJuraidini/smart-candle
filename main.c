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

#define BUILTIN_LED2    BIT6  // P1.6
#define BUILTIN_S2      BIT3  // P1.3

// RGB LED pins
#define RED_LED     BIT2    // P1.2
#define GREEN_LED   BIT1    // P2.1
//#define BLUE_LED    BIT4    // P2.4

// TILT SENSOR
#define TILT    BIT4        // P1.4

// increase MAX_RAND and LED_ON_FACTOR to make LED flicker slower
#define MAX_RAND 20
#define LED_ON_FACTOR 18

#define ADC_BUSY        ADC10CTL1 & BUSY
#define MIC_SAMPLE_SIZE 150     //150
#define MIC_THRESHOLD       430     //430// adjusts when LED turns off
#define FLICKER_THRESHOLD   100     //200// adjusts when LED flicker from weak breath
#define EXTRA_FLICKER       15      // adjusts amount of flicker (higher = more flicker)

#define SHAKE_SEQUENCE_TIMEOUT  600    //500 // adjusts max time between shakes
#define SINGLE_SHAKE_TIME       350    //250 // adjusts time a single shake takes (up then down)
                                        // 1000 ~= 1 second

// pwm = 0 on 100%, 100 on 0%
// flame color: red 5, green 90, blue 100

/** flameColor
 * @summary adjusts pwm wave of red and green pins of RGB
 * @param red_percent = red led duty cycle
 * @param green_percent = green led duty cycle
 * @return nothing
 */
void flameColor(int red_percent, int green_percent);

/** pwmInit
 * @summary initializes pwm pins and timer modules used to generate waves
 * @param none
 * @return none
 */
void pwmInit(void);

/** adcInit
 * @summary initializes ADC10 analog to digital converter module
 * @param none
 * @return non
 */
void adcInit(void);

/** readA0
 * @summary activates the ADC to read the analog value on pin P1.0
 * @param none
 * @return 10-bit integer value from ADC
 */
int readA0(void);

/** switchInit
 * @summary initializes built-in switch S2 with interrupt capability
 * @param none
 * @return none
 */
void switchInit(void);

volatile unsigned long long cycles = 0;
volatile int led_status = 1; // 1 if on, 0 if off
volatile int tilt_flag = 0; // 1 on interrupt, cleared 0 in main loop

int main(void)
{
    int red_duty = 95, green_duty = 5;
    int rand_on;
    unsigned int adc_value = 0;
    unsigned int adc_count = 0, adc_average = 0;
    unsigned long adc_sum = 0;
    unsigned int led_extra_flicker_cycles = 0;
    unsigned long tilt_disabled_count = 0;
    int tilt_count = 0;
    unsigned long long last_tilt_time = 0;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= BUILTIN_LED2;

	adcInit();
	pwmInit();
	switchInit();

	P1DIR &= ~TILT;
	P1IE |= TILT;
	P1IES |= TILT;
	P1IFG &= ~TILT;

	__enable_interrupt();

	while(1)
	{
	    if(led_status == 1)
	    {
	        // ** led on **
	        if(led_extra_flicker_cycles > 0)
	        {
	            // flicker led if last breath was too weak to turn off
	            led_extra_flicker_cycles--;
	            rand_on = rand() % (MAX_RAND + (led_extra_flicker_cycles % EXTRA_FLICKER));
	            green_duty = 3;
	        }
	        else
	        {
                // generate random value for LED flicker
                rand_on = rand() % MAX_RAND;
                green_duty = 5;
	        }

            // turn off LED based on random value or tilt sensor
            if( (rand_on > LED_ON_FACTOR) ) flameColor(0, 0);
            // or set correct pwm
            else flameColor(red_duty, green_duty);

            // read mic value
            adc_value = (unsigned int)readA0();

            if(adc_count >= MIC_SAMPLE_SIZE){
                adc_average = adc_sum / adc_count;
                // check if blow was strong enough
                // turn off candle if it is
                if(adc_average > MIC_THRESHOLD)
                {
                    // turn off led
                    led_status = 0;
                }
                else if(adc_average > FLICKER_THRESHOLD)
                {
                    // make candle flicker more
                    led_extra_flicker_cycles = MIC_SAMPLE_SIZE;
                }
                adc_sum = 0;
                adc_count = 0;
            }
            adc_count++;
            adc_sum += adc_value;
	    }
	    else
	    {
	        // ** led off **
	        flameColor(0, 0);
	        // add delay to match time of bigger 'if' statement
	        // this is needed for shake calibration to work in both conditions
	        __delay_cycles(700);
	    }

	    // ** shake detection **
	    if(tilt_flag)
        {
            // disable tilt switch interrupts for 0.2 seconds
            tilt_disabled_count = SINGLE_SHAKE_TIME;
            P1IE &= ~TILT;
            P1IFG &= ~TILT;
            // acknowledge shake
            tilt_flag = 0;
            tilt_count++;
            // mark time of tilt
            last_tilt_time = cycles;
        }

        if(tilt_disabled_count)
        {
            tilt_disabled_count--;
            // enable interrupt if counter is done
            if(tilt_disabled_count < 1)
            {
                // re-enable tilt sensor interrupts
                // clear flags since they may have been set when modifying registers
                P1IFG &= ~TILT;
                P1IE |= TILT;
                P1IFG &= ~TILT;
                // check if there has been 3 shakes
                if(tilt_count >= 3)
                {
                    if(led_status == 0) led_status = 1;
                    else if(led_status == 1) led_status = 0;
                    tilt_count = 0;
                }
            }
        }

        // check time between shakes to see if sequence timed out
        if(cycles - last_tilt_time > SHAKE_SEQUENCE_TIMEOUT)
        {
            tilt_count = 0;
        }

        // ** end shake detection **

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
    P2DIR |= GREEN_LED;  // + BLUE_LED;
    P2SEL |= GREEN_LED;  // green 2.1 pwm
    //P2OUT &= ~BLUE_LED;  // blue 2.4 off

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

void switchInit(void)
{
    P1DIR &= ~BUILTIN_S2;
    P1REN |= BUILTIN_S2;
    P1IE |= BUILTIN_S2;
    P1IES |= BUILTIN_S2;
    P1IFG &= ~BUILTIN_S2;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BUILTIN_S2)
    {
        led_status = 1;
        P1IFG &= ~BUILTIN_S2;
    }

    if(P1IFG & TILT)
    {
        tilt_flag = 1;
        P1OUT ^= BUILTIN_LED2;
        P1IFG &= ~TILT;
    }

}
