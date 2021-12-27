#include <avr/io.h>
#include <avr/interrupt.h>
#include "util/delay.h"
/* global variables contain the counts of the timer */
unsigned char sec = 0;
unsigned char min = 0;
unsigned char hour = 0;
/* Interrupt Service Routine for timer1 compare mode */
ISR(TIMER1_COMPA_vect)
{
	sec++;
	if (sec==60){//incrementing the minutes
		min++;
		sec=0;
	}
	if (min==60){//incrementing the hours
		hour++;
		sec=0;
		min=0;
	}
	if (hour==12){//starting new 12 hours
		hour=0;
		sec=0;
		min=0;
	}
}

void TIMER1_init(void){
	TCNT1 = 0;		/* Set timer1 initial count to zero */
/*Ftimer = CPU Frequency/Prescalar, Ftimer = 1MHz/256 = 3906.25Hz ,Ttick = 1/ 3906.25Hz = 0.256 msec, Ttotal = 0.256 msec X 65536 = 16.7 seconds */
	/*We just need 1 sec so, 1 sec /0.256 msec = 3906.25 */
	OCR1A = 3906;
	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Interrupt */

	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */
	TCCR1A = (1<<FOC1A);//Non PWM mode

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/256 CS10=0 CS11=0 CS12=1
	 */
	TCCR1B = (1<<WGM12) |(1<<CS12);
}
void INT0_init(void){
	SREG&=~(1<<7);// Disable interrupts by clearing I-bit
	DDRD&=~(1<<2);//push button (input)
	PORTD|=(1<<2);//enable internal pull up resistor
	MCUCR|=(1<<ISC01);
	MCUCR&=~(1<<ISC00);//falling edge : ISC01=1,  ISC00=0
	GICR|=(1<<INT0);//enable interrupt INT0
	SREG|=(1<<7); // Enable interrupts by setting I-bit
}
/* Interrupt Service Routine for INT0 to reset all variables */
ISR(INT0_vect){
	sec=0;
	min=0;
	hour=0;
	GIFR|=(1<<6);//clear flag of INT0
}
void INT1_init(void){
	SREG&=~(1<<7);// Disable interrupts by clearing I-bit
	DDRD&=~(1<<3);//D3 as input
	MCUCR|=(1<<ISC10)|(1<<ISC11);//rising edge
	GICR|=(1<<INT1);//enable interrupt INT1
	SREG|=(1<<7);// Enable interrupts by setting I-bit
}
/* Interrupt Service Routine for INT1 to stop timer */
ISR(INT1_vect){
	TCCR1B &=~ (1<<CS10)& (1<<CS11)& (1<<CS12);
	GIFR|=(1<<7);
}
void INT2_init(void){
	SREG&=~(1<<7);
	DDRB&=~(1<<2);
	PORTB |= (1<<2);//enable internal pull up resistor
	MCUCR&=~(1<<ISC2);//falling edge ISC2=0
	GICR|=(1<<INT2);
	SREG|=(1<<7);
}
/* Interrupt Service Routine for INT2 to resume timer*/
ISR(INT2_vect){
	TIMER1_init();
	GIFR|=(1<<5);
}
int main(void)
{
	DDRC|=0x0F;//seven segment as an output
	PORTC|=0X00;//initializing 7 segment
	DDRA=0xFF;//pins to enable one by one 7 segment (output)
	PORTA=0xFF;//initializing pins
	SREG |= (1<<7); /* Enable global interrupts in MC */
	INT0_init();
	INT1_init();
	INT2_init();
	TIMER1_init();
	while(1)
	{
		PORTA=0x01;//enable first 7 segment
		PORTC=sec%10;//to take first digit only in seconds variable
		_delay_ms(2);//delay as small as possible so our eyes don't notice it

		PORTA=(1<<1);//enable second 7 segment
		PORTC=sec/10;//to take second digit only in seconds variable
		_delay_ms(2);

		PORTA=(1<<2);
		PORTC=min%10;
		_delay_ms(2);

		PORTA=(1<<3);
		PORTC=min/10;
		_delay_ms(2);


		PORTA=(1<<4);
		PORTC=hour%10;
		_delay_ms(2);

		PORTA=(1<<5);
		PORTC=hour/10;
		_delay_ms(2);
	}
}
