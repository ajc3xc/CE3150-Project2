/*
	Comp Eng 3150 Project 2
	Members: Adam Camerer, Evan Parrish, Ethan Mollet, Jerrett Martin
	Date: 12/5/2023
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <time.h>

//8 levels seems reasonable for a demo
//theoretically, 16 levels is possible with the middle right led
//but I need to demonstrate what happens when the timer overflows
#define MAX_LEVEL 8
#define	LCD_DATA  PORTC
#define	LCD_COMMAND  PORTD

#define TIMER_FREQUENCY 1
#define TIMER_PRESCALER 1024
#define TIMER_VALUE 7813



int SEQUENCE[MAX_LEVEL];
int LVL = 1;
int INDEX = 0;
volatile int DONE = 0;
int timerOverflows[3]; //array checking whether the timer has overflowed or not

//


void generate_simon_pattern(){
	srand(time(NULL));
	for(int i=0 ; i<MAX_LEVEL ; i++){
		SEQUENCE[i] = (rand() % 4) + 1; //generate a random number from 1-4 inclusive
	}
}

void delay()
{
	_delay_ms(700);
}
/*
void LCD_commander( unsigned char command )
{
	LCD_DATA = command;
	LCD_COMMAND &= ~ (1<<7);	//RS=0
	LCD_COMMAND &= ~ (1<<6);	//RW=0 write
	LCD_COMMAND |= (1<<5);	//ENABLE=1
	_delay_us(1);
	LCD_COMMAND &= ~ (1<<5);	//ENABLE=0
	_delay_us(100);
}

void LCD_char_datashow( unsigned char data )
{
	LCD_DATA = data;
	LCD_COMMAND |= (1<<7);	//RS=1 means data
	LCD_COMMAND &= ~ (1<<6);	//RW=0 write
	LCD_COMMAND |= (1<<5);	//ENABLE=1
	_delay_us(1);
	LCD_COMMAND &= ~ (1<<5);	//ENABLE=0
	_delay_us(100);
}

void LCD_initializer()
{
	DDRC = 0xFF;
	DDRD = 0xF0; // make PORTD output for LCD commands
	
	LCD_COMMAND &=~(1<<5);	//ENABLE=0
	_delay_us(2000);
	LCD_commander(0x38);		//char size 5*7 16 char in 2 line
	LCD_commander(0x0E);		//turn lcd on
	LCD_commander(0x01);		//clean lcd
	_delay_us(2000);
	LCD_commander(0x06);		//move cursor to proper position
}

void LCD_clean(){
	LCD_commander(0x01);		//clean lcd
	_delay_us(2000);
	LCD_commander(0x06);		//move cursor to proper position
}

void LCD_string_datashow(char* string )
{
	unsigned char i = 0;
	while(string[i]!=0)
	{
		LCD_char_datashow(string[i]);
		i++ ;
	}
}

ISR (INT2_vect)
{
	int choosen = 0;
	if(PINB & (1 << 4))
	{
		PORTA |= (1<<0);
		choosen = 1;
	}
	else if(PINB & (1 << 5))
	{
		PORTA |= (1<<1);
		choosen = 2;
	}
	else if(PINB & (1 << 6))
	{
		PORTA |= (1<<2);
		choosen = 4;
	}
	else if(PINB & (1 << 7))
	{
		PORTA |= (1<<3);
		choosen =8;
	}
	delay();
	PORTA = 0x00;
	if(SEQUENCE[INDEX] != choosen)
	{
		LCD_clean();
		LCD_string_datashow("GAME IS UP");
		LCD_commander(0xC0);	//next line
		char level[50];
		sprintf(level,"YOUR LEVEL: %d", LVL-1);
		LCD_string_datashow(level);
		LVL = 0;
		INDEX = 0;
		DONE = 1;
		_delay_ms(1000);
		return ;
	}
	else
	{
		INDEX++;
	}
	if(INDEX == LVL)
	{
		DONE = 1;
		LCD_clean();
		LCD_string_datashow("RIGHT");
		LCD_commander(0xC0);	//next line
		LCD_string_datashow("NEXT LEVEL");
		delay();
		INDEX = 0;
		return;
	}
	return;
}
*/

void initialize_ports()
{
	DDRA = 0x00; //PINA input
	DDRD = 0xFF; //DDRD input
	DDRE = 0b00110000; //outputs PINE bits 5, 4, inputs bit 6
	
	//0 means on, 1 means off
	PORTA |= 0xFF;
	PORTD |= 0xFF; //turn off portd
	PORTE |= 0b00110000; //turn off port e
	
	//turn on all LEDs
	//PORTD &= 0x00;
	//PORTE &=  0b11001111;
	
	return;
}

//choose which led to light up
//used when selecting a button
void light_simon_led(int led_to_light)
{
	//0 means on, 1 means off
	//PORTD |= 0xFF; //turn off portd
	//PORTE |= 0b00110000; //turn off port e
	
	//turn off simon specific leds
	PORTD |= (1<<PORTD7);
	PORTD |= (1<<PORTD6);
	PORTD |= (1<<PORTD4);
	PORTE |= (1<<PORTE5);
	
	switch(led_to_light)
	{
		case 1: //turn on port D bit 7 (9th led)
			PORTD &= ~(1<<PORTD7);
			break;
		case 2: //turn on port D bit 1 (8th led)
			PORTD &= ~(1<<PORTD6);
			break;
		case 3: //turn on port D bit 4 (6th led)
			PORTD &= ~(1<<PORTD4);
			break;
		case 4: //turn on port E bit 5 (4th led)
			PORTE &= ~(1<<PORTE5);
			break;
		default:
			break;
	}
	return;
}



//display the current level on D1-D3 as 3 bit value
void display_level_leds()
{
	//turn off leds 1-3
	PORTD |= (1<<PORTD0);
	PORTD |= (1<<PORTD1);
	PORTD |= (1<<PORTD2);
	
	//this logic only works if there is up to 8 levels, and there isn't a level 0
	//as there is only 3 leds to display the level
	if (LVL==0 || LVL > MAX_LEVEL)	return;
	//display current levels on leds
	PORTD &= ~(LVL-1);
	
}

int timer_counter = 1;
void time_delay() {
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCNT1 = TIMER_VALUE;
	TIMSK1 |= (1 << TOIE1);
	sei();
	
	while(timer_counter < 1);
}

ISR(TIMER1_OVF_vect) {
	timer_counter++;
	if (timer_counter >= 1) {
		// Disable Timer/Counter1 overflow interrupt
		TIMSK1 &= ~(1 << TOIE1);
		PORTD &= ~(1<<PORTD4);
	}
	TCNT1 = TIMER_VALUE;
}

#define HALF_SECOND_TIMER_VAL 0
#define HALF_SECOND_ITERATIONS 15
//generates a half second delay on timer 0
// (255 * 1024 * 15) / (8 * 10^6) = .4896 seconds
void half_second_delay()
{
	TCNT0 = HALF_SECOND_TIMER_VAL;
	TCCR0A = 0x00;
	TCCR0B |= 0x05; // normal clock, prescaler 1024
	
	int counter = 0;
	while(counter < HALF_SECOND_ITERATIONS)
	{
		//poll until timer overflows
		while (!(TIFR0 & (1<<TOV0)));
		TCNT0 = HALF_SECOND_TIMER_VAL; //reset tcnt0
		TIFR0 = 1<<TOV0; //need to reset the overflow flag bit
		counter++;
	}
	
	//turn off timer0
	TCCR0A = 0x00;
	TCCR0B = 0x00;
	TIFR0 = 1<<TOV0;
	
}

//turn off all the lights
void turn_off_leds()
{
	PORTD = 0xFF;
	PORTE |= (1<<PORTE5);
}


//plays the speaker for 1 second
void play_speaker(int divisor)
{
	//divisor must be 2^n
	if (!(divisor && !(divisor & (divisor-1)))) return;
	 
	//const int TIMER_VAL = 256/divisor - 1
	const int TIMER_VAL = (255 - ((256/divisor) - 1));
	TCNT2 = TIMER_VAL;
	
	TCCR2A = 0x00; //normal clock
	TCCR2B |= 0x07; // prescaler 1024
	
	int counter = 0;
	const int MAX_ITERATIONS = 30 * divisor;
	//total time = 1024 * (255 - (256/divisor - 1)) * 30 * divisor
	while(counter < MAX_ITERATIONS)
	{
		//poll until timer overflows
		while (!(TIFR2 & (1<<TOV2)));
		PORTE ^= (1<<PORTE4); //toggle PORTE6
		//PORTE ^= (1<<PORTE5); //toggle PORTE5
		TCNT2 = TIMER_VAL; //reset tcnt2
		TIFR2 = 1<<TOV2; //need to reset the overflow flag bit
		counter++;
	}
	
	//turn off timer0
	TCCR2A = 0x00;
	TCCR2B = 0x00;
	TIFR2 = 1<<TOV2;
}

//call the function if the game was lost
void lose_game()
{
	//turn off LEDs
	turn_off_leds();
	
	//display an x pattern on the LEDs
	PORTD &= ~(1<<PORTD7);
	PORTD &= ~(1<<PORTD5);
	PORTE &= ~(1<<PORTE5);
	PORTD &= ~(1<<PORTD2);
	PORTD &= ~(1<<PORTD0);
	play_speaker(2);
	
	//turn off the LEDs again
	turn_off_leds();
	
	
	//reset to level 1
	LVL = 1;
	
	//regenerate pattern for simon board
	generate_simon_pattern();
	return;
}

//call function if the game was won
void win_game()
{
	//turn off LEDs
	turn_off_leds();
	
	//turn on all LEDs
	PORTD = 0x00;
	PORTE = ~(1<<PORTE5);
	play_speaker(64);
	
	//turn off LEDs
	turn_off_leds();
	
	//reset to level 1
	LVL = 1;
	
	//regenerate pattern for simon board
	generate_simon_pattern();
	return;
}

//plays the sequence of leds you need to get right
void play_sequence()
{
	for(int i=0; i<LVL; i++){
		light_simon_led(SEQUENCE[i]);
		play_speaker(4);
		light_simon_led(0);
		half_second_delay();
	}
}

int main(void)
{
	initialize_ports();
	generate_simon_pattern();
	
	LVL = MAX_LEVEL;
	
	//play_sequence();
	
	while(1)
	{
		//int value = (1<<PINE6);
		if (!(PINE & (1 << PINE6))) PORTE &= ~(1<<PORTE5);
		else if (PINE & (1 << PINE6)) PORTE |= (1<<PORTE5);
		//if (PINA7 == 1)	PORTD &= ~(1<<PORTD7);
		PORTD = PINA;
		half_second_delay();
	}
	
	while(1);
	
	/*
	sei();				// set interrupt enable
	initialization_function();
	LCD_initializer();
	exit;
	
	
	while (LVL <= MAX_LEVEL)
	{
		LCD_clean();
		LCD_string_datashow("LEARN PATTERN");
		for(char i=0 ; i<LVL ; i++){
			delay();
			PORTA = SEQUENCE[i];
			delay();
			PORTA = 0x00;
		}
		delay();
		PORTA = 0x00;
		LCD_clean();
		LCD_string_datashow("YOUR TIME");
		LCD_commander(0xC0);
		char level[50];
		sprintf(level,"LEVEL: %d", LVL-1);
		LCD_string_datashow(level);
		
		while(DONE == 0);
		DONE = 0;
		
		LVL++;
	}
	LCD_clean();
	LCD_string_datashow("WOW IMPOSSIBLE!");
	while(1);
	*/
}