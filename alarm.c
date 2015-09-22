#include <msp430.h> 
#include <time.h>
#include <stdio.h>
#include <string.h>


unsigned int hour;
unsigned int minute;
unsigned int halfsecond;
unsigned int alarmhour;
unsigned int alarmminute;


void updateminute();
void updatehour();
void updateaminute();
void updateahour();
void InitializeButton();
void buzz();
void stop();
void snooze();

# define R1 0x00
# define R2 0x02
# define R3 0x04
# define R4 0x06
# define BUTTON 0x03
# define SONE 0x04
# define STWO 0X08
# define ATRIG 0x10
# define STHREE 0x20



/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer


	/**
	 * calibrate clock
	 */
	BCSCTL1 = CALBC1_1MHZ;                    // Set range
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz

    P1DIR = 0xFF;  // initializing P1 to all outputs
    P2DIR = ATRIG;  //set P2.5 to be the alarm

	TACCR0 = 62500;   // count to 50,000/ 0.5s
	TACTL = TASSEL_2 + MC_1 + ID_3;// set SMCLK as source, divide by 8, upmode
	TACCTL0 = CCIE;



	InitializeButton();
	__enable_interrupt();

	// Initialize current time
	hour = 14;
	minute = 45;
	halfsecond = 0;
	
   //Initialize alarm time
	alarmhour = 14;
	alarmminute = 44;

	// initial display of time
	updateminute();
	updatehour();



	__bis_SR_register(LPM0_bits + GIE); // turn off CPU


}

/*
 * Timer A interrupt to update minute/hour field
 */

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (){


    if (halfsecond + 1 < 120)           //update second/hour/minute field
		halfsecond ++;
	else
		{halfsecond = 0;
		if (minute + 1 < 60)
			{minute++;
			}

		else
			{minute = 0;
			updateminute();
			if (hour + 1 < 24)
				{hour++;
				}
			else{
				hour = 0;
			}
	    }
   }                                //

	if (hour == alarmhour){        //
		if (minute == alarmminute)
			buzz();
	                      }       // check if alarm time is reached

	if (P2IN & SONE){            //if S1 turned on, display alarm time
		updateaminute();
		updateahour();
					 }
	else {                        //if S1 turned off, display current time
		updateminute();
		updatehour();
	}
	if (P2IN & STWO){
		stop();
		if (P2IN & STHREE){

		        snooze();
			                       }
	}





	TACCTL0 &= 0xfffe;
}
/*group of fuctions which put the field values on to the
 * screen using 7 segemetn decoder, 2-4 line decoder, and strobe.
 */

void updateminute(){

  int mten = minute/10;


  int mone = minute - (10*mten);

  char d3 = (mten << 3) + R2;
        P1OUT = d3 + 0x01;
        P1OUT = d3;
		P1OUT ^= 0x01;

  char d4 = (mone << 3) + R1;
        P1OUT = d4 + 0x01;
	    P1OUT = d4;
	    P1OUT ^= 0x01;

}

void updateaminute(){

  int amten = alarmminute/10;


  int amone = alarmminute - (10*amten);

  char d3 = (amten << 3) + R2;
        P1OUT = d3 + 0x01;
        P1OUT = d3;
		P1OUT ^= 0x01;

  char d4 = (amone << 3) + R1;
        P1OUT = d4 + 0x01;
	    P1OUT = d4;
	    P1OUT ^= 0x01;

}

void updatehour(){

	int hten = hour/10;
	int hone = hour - (10*hten);

	char d1 = (hten << 3) + R4;
        P1OUT = d1 + 0x01;
        P1OUT = d1;
		P1OUT ^= 0x01;
	char d2 = (hone << 3) + R3;
	    P1OUT = d2 + 0x01;
	    P1OUT = d2;
	    P1OUT ^= 0x01;

}

void updateahour(){

	int ahten = alarmhour/10;
	int ahone = alarmhour - (10*ahten);

	char d1 = (ahten << 3) + R4;
        P1OUT = d1 + 0x01;
        P1OUT = d1;
		P1OUT ^= 0x01;
	char d2 = (ahone << 3) + R3;
	    P1OUT = d2 + 0x01;
	    P1OUT = d2;
	    P1OUT ^= 0x01;

}
/* set up button interrupt initialization
 *
 */

void InitializeButton(void)                 // Configure Push Button
{
  P2DIR &= ~(BUTTON + SONE + STWO);
  P2OUT |= BUTTON ;
  P2REN |= BUTTON ;
  P2IES |= BUTTON;
  P2IFG &= ~BUTTON;
  P2IE |= BUTTON + SONE + STWO;
}
/* button interrupt to increase value of hour/minute
 * upon pressing the button
 */

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {


	if(P2IFG & 0x01) {

        if (P2IN & SONE){
        	if (alarmminute + 1 < 60)
	    	alarmminute++;
        	else
	    	alarmminute = 0;

        	updateaminute();
                       }
        else {

        	if (minute + 1 < 60)
        		minute++;
        	else
        		minute = 0;

            halfsecond = 0;
        	updateminute();
        }


                     }

	if(P2IFG & 0x02) {

		if (P2IN & SONE){
			if (alarmhour + 1 < 24)
				alarmhour++;
			else
				alarmhour = 0;

			updateahour();
		}
		else {
			if (hour + 1 < 24)
				hour++;
			else
				hour = 0;

			halfsecond = 0;
			updatehour();
		}

	                }



	P2IFG &= ~BUTTON;

}
/* PWM cycle using timer A2
 *
 */

void buzz(void){
	P2SEL |= ATRIG;
	TA1CTL = TASSEL_2 + MC_1;
	TA1CCR0 = 1000-1;
	TA1CCTL2 = OUTMOD_7;
	TA1CCR2 = 250;

}
/*disable PWM to turn off buzzer
 *
 */
void stop (void){

	P2SEL &= ~ATRIG;
}


/* increase the alarm minute field by 4 to achieve snoozing
 *
 */
void snooze(void){
	if (alarmminute + 2 < 60)
				{alarmminute = alarmminute + 4;
				}

	else{
		alarmminute = 0;
		if (alarmhour + 1 < 24)
					{alarmhour++;
					}
		else{
			alarmhour = 0;
			}
		}
}

