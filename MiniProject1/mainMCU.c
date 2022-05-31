/*
 * mainMCU.c
 *
 *  Created on: 2 May 2021
 *      Author: Solwan Shokry Ahmed
 */

#include<avr/io.h>
#include<avr/interrupt.h>
#define F_CPU 1000000			//F_CPU=1MHz
#include<util/delay.h>
#include"MCU.h"


unsigned char seg[6],i;
unsigned int secCounter=0,minCounter=0,hoursCounter=0;


									//Timer1//

/*FCPU = 1 MHz*/
/* Configure the Timer1 to:
 * 1- clear timer on output compare match mode (CTC mode) using compare register A.
 * 2- operate with an external clock frequency of F_CPU/1024 resulting a tick after each passing 1 mSec.
 * 3- set the compare value to 1000 to get a compare match after each passing second (1000 ticks of the timer).
 */

void Timer1_CTC_A_Mode_Init(void)
{
	/*Configuring Timer1A, Timer1B control register
	 * 1) Clear on compare match,Non-PWM mode COM1A1=1 &	COM1A0=0
	 * 2) Force output compare only in register A so bits FOC1A=1 & FOC1B=0
	 * 3) CTC Mode WGM13=0 & WGM12=1 in control register B & WGM11=0 & WGM10=0 in control register A
	 * 4) Clock select with pre-scalar F_CPU/1024 value in TCCR1B:
	 * 		CS12=1 & CS11=0 & CS10=1
	 */
	TCCR1A=(1<<COM1A1)|(1<<FOC1A);
	TCCR1B=(1<<CS12)|(1<<CS10)|(1<<WGM12);

	//Set the timer's initial value to zero
	TCNT1=0;

	//SET THE COMPARE REGISTER A VALUE TO THE NUMBER OF COUNTS
	OCR1A=976;

	/* Enable the global interrupt requests in the MCU status register (SREG) by setting the I-bit (bit no.7)*/
	SREG |= (1<<7);

	//Set the  Bit OCIE1A, to Enable Output Compare A Match Interrupt in Timer 1 interrupt mask register
	TIMSK |= ( 1 << OCIE1A );

	//Global variable to hold the number of compare matches of counter/timer1 with an initial value equal to ZERO
	//secCounter=0;
}


ISR(TIMER1_COMPA_vect){
	//Set the I bit in the status register to enable the global interrupts in case any external interrupt happened at the same time
	//SREG|=(1<<7);

	secCounter++;

}
								//Reset function//


void Reset (void)			//function to reset the stop watch
{
	//Re-set//
	SREG &=~ (1<<7);				//disable the global interrupts
	TIMSK&= ~(1 << OCIE1A);			//disable the Timer's interrupts

	//1) The variables holding the number of seconds, minutes and hours must be re-set to zero
	secCounter=0;
	minCounter=0;
	hoursCounter=0;

	for(i=0;i<=5;i++)		//the array holding the digits to be displayed is also re-set to zero
	{
		seg[i]=0;
	}

	//2) The Timer1 counter register must be re-set to zero >> TCNT1=0
	TCNT1=0;

	TIMSK|= (1 << OCIE1A);			//enable the timer's interrupts
	SREG |= (1<<7);					//enable global interrupts
}



							//INT2//
void INT0_Init(void){

	//CONFIGURE pin 2 in PORTD as an input pin for INT0 with internal pull-up resistor
	DDRD&=~(1<<2);		//PD2 is now an input pin
	PORTD|=(1<<2);		//Internal pull-up resistor is now activated

	/* Enable the global interrupt requests in the MCU status register (SREG) by setting the I-bit (bit no.7)*/
	SREG |= (1<<7);


	/* Enable the external interrupt request of INT0 by setting bit 6 in the general interrupt control register*/
	GICR|=(1<<INT0);

	/*Enable the external interrupt INT0 sense in the MCU-control register
	 * working mode: Falling Edge on pin INT0 generates an external interrupt request ISC01=1 & ISC00=0*/
	MCUCR&=~(1<<ISC00);		//ISC00=0
	MCUCR|=(1<<ISC01);		//ISC01=1

}

/* When INT0 occurs, we wish to re-set the counter meaning two procedures
 * 1) The variables holding the number of seconds, minutes and hours must be re-set to zero
 * 2) The Timer1 counter register must be re-set to zero >> TCNT1=0
 * */
ISR(INT0_vect)
{
			//Re-set//

	Reset();
}

								//INT1//
void INT1_Init(void){

	//Configure pin 3 in PORTD as an input pin for INT1 with external pull-down resistor
	DDRD&=~(1<<3);


	/* Enable the global interrupt requests in the MCU status register (SREG) by setting the I-bit (bit no.7)*/
	SREG |= (1<<7);

	/* Enable the external interrupt request of INT1 by setting bit 7 in the general interrupt control register*/
	GICR|=(1<<INT1);

	/*Enable the external interrupt INT1 sense in the MCU-control register
	 * working mode: Rising Edge on pin INT1 generates an external interrupt request ISC11=1 & ISC10=1*/
	MCUCR|=(1<<ISC11)|(1<<ISC10);

}

/* when INT1 occurs, we wish to pause the stop watch. this is done by blocking the clock source of Timer1
 * by clearing the bits CS12=0 & CS11=0 & CS10=0 in TCCR1B*/
ISR(INT1_vect){

					//Pause//

	SREG&=~(1<<7);			//disable the global interrupts

	//THE CLOCK SOURCE IS DISABLED OF THE TIMER, THE TIMER STOPS COUNTING
	TCCR1B&=~(1<<CS12);
	TCCR1B&=~(1<<CS11);
	TCCR1B&=~(1<<CS10);

	TIMSK&= ~(1 << OCIE1A); 		 //Turn off the timer interrupts

	SREG|=(1<<7);			//enable the global interrupts
}

										//INT2//

void INT2_Init(void){

	//Configure pin 2 in PORTB as an input pin for INT2 with internal pull-up resistor
	DDRB&=~(1<<2);		//PB2 is now an input pin
	PORTB|=(1<<2);		//internal pull-up resistor is now activated

	/* Enable the global interrupt requests in the MCU status register (SREG) by setting the I-bit (bit no.7)*/
	SREG |= (1<<7);

	/* Enable the external interrupt request of INT2 by setting bit 5 in the general interrupt control register*/
	GICR|=(1<<INT2);

	/*Enable the external interrupt INT2 sense in the MCU-control register
	 * working mode: Falling Edge on pin INT2 generates an external interrupt request ISC2=0 */
	MCUCSR&=~(1<<ISC2);
}

/*when INT2 occurs, we wish to resume our functionality. this is done by re-enabling the clock source on our timer
 * by the same pre-scalar value of F_CPU/1024 with clock select mode CS12=1 & CS11=0 & CS10=1 in TCCR1B */
ISR(INT2_vect){

				//Resume//

	//THE CLOCK SOURCE IS ENABLED OF THE TIMER, THE TIMER STARTS COUNTING

	TCCR1B|=(1<<CS12)|(1<<CS10);		//CS12=1 &  CS10=1
	TCCR1B&=~(1<<CS11);					//CS11=0

	TIMSK|= (1 << OCIE1A); 		 //Turn On the timer interrupts

}

									//Display function//

void display (void)
{
	if(secCounter<60)						//The number of seconds to be displayed is not greater than 59
		{
			seg[0]= ( secCounter % 10 );		//the first digit of the number in the corresponding display variable in the array
			seg[1]= ( secCounter / 10);			//the second digit of the number in the corresponding display variable in the array
		}
	else		//if The number of seconds to be displayed is greater than 59, then we counted a complete minute
		{
			if(minCounter<60)  			//The number of minutes to be displayed is not greater than 59

			{
				//new starting minute >> reset the seconds counter  to be 00>> seg[1]=0 & seg[0]=0
				secCounter=0;
				seg[0]= 0;
				seg[1]= 0;
				minCounter++;					//increment the minutes counter
				seg[2]= (minCounter % 10);		//the first digit of the number in the corresponding display variable in the array
				seg[3]= (minCounter / 10 );		//the second digit of the number in the corresponding display variable in the array
			}

			else  	//if The number of minutes to be displayed is greater than 59, then we counted a complete hour
			{
				if(hoursCounter<100)		//The number of minutes to be displayed is not greater than 99
				{
					//new starting hour >> reset the seconds counter  to be 00>> seg[1]=0 & seg[0]=0
					//and the minutes counter to be 00>> seg[3]=0 & seg[2]=0
					secCounter=0;
					seg[0] = 0;
					seg[1] = 0;
					minCounter=0;
					seg[2] = 0;
					seg[3] = 0;
					hoursCounter++;						//increment the hours counter
					seg[4]= ( hoursCounter % 10 );		//the first digit of the number in the corresponding display variable in the array
					seg[5]= ( hoursCounter / 10 );		//the second digit of the number in the corresponding display variable in the array
				}
				else
				{
					Reset();							//Re-set the stop watch
				}
			}
		}
	for (i=0;i<=5;i++)			//Loop on the display digits array seg[6]
	{
		/*activate the corresponding 7-segment display by setting the corresponding output port in PORTA to 1
		 * to select the common anode segment
		 */
		PORTA =(1<<i);

		/* Output the digit's value on PORTC without affecting the any possible operations connected on the other 4pins of the PORT*/
		PORTC = (PORTC & 0xF0) | (seg[i] & 0x0F);


		_delay_ms(5);		//Delay for 5 milli-seconds

		/*De-activate the corresponding 7-segment display by re-setting the corresponding output port in PORTA to 0
		 * to dis-select the common anode segment and continue the loop to display the next one and so on
		 */
		PORTA &=~(1<<i);
	}
}

 	 	 	 	 	 	 	 	 	 //Main Program//
int main(void){

	/*Initialization of IO-ports, each with the corresponding configurations*/

	//configure PORTA pins 0-1-2-3-4-5 as an output pins for the 7-segment common anode (internal n-P-n transistors) selection
	DDRA|=(1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5);

	//configure PORTC pins 0-1-2-3 as an output pins for the 7-segment decoder
	DDRC|=(1<<0)|(1<<1)|(1<<2)|(1<<3);

	INT0_Init();			//initialize external interrupt  INT0 >> responsible for re-setting the stop watch
	INT1_Init();			//initialize external interrupt  INT1 >> responsible for pausing the stop watch
	INT2_Init();			//initialize external interrupt  INT2 >> responsible for resuming the stop watch

	Timer1_CTC_A_Mode_Init();	//initialize the Timer Timer1 working with CTC-mode, using compare register A

	/* Enable the global interrupt requests in the MCU status register (SREG) by setting the I-bit (bit no.7)*/
	SREG |= (1<<7);

	while(1)
	{
		display();			//display on the 7-segment display block (having 6 7-segment displays)
	}
}



