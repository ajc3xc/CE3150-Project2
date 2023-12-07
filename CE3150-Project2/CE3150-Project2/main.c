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

#define MAX_LEVEL 250
#define	LCD_DATA  PORTC
#define	LCD_COMMAND  PORTD


char SEQUENCE[MAX_LEVEL];
int LVL = 1;
int INDEX = 0;
volatile int DONE = 0;


void initialization_function(){
	srand(time(NULL));
	for(int i=0 ; i<MAX_LEVEL ; i++){
		SEQUENCE[i] = 1 << (rand() % 4);
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
	DDRD = 0xFF;
	DDRE = 0b00110000;
	
	//0 means on, 1 means off
	PORTD |= 0xFF; //turn off portd
	PORTE |= 0b00110000; //turn off port e
	
	//turn on all LEDs
	//PORTD &= 0x00;
	//PORTE &=  0b11001111;
	
	return;
}

//choose which led to light up
void light_simon_led(int led_to_light)
{
	//0 means on, 1 means off
	PORTD |= 0xFF; //turn off portd
	PORTE |= 0b00110000; //turn off port e
	
	switch(led_to_light)
	{
		case 1: //turn on port D bit 0 (1st led)
			PORTD &= ~(1<<PORTD7);
			break;
		case 2: //turn on port D bit 1 (2nd led)
			PORTD &= ~(1<<PORTD6);
			break;
		case 3: //turn on port D bit 4 (4th led)
			PORTD &= ~(1<<PORTD4);
			break;
		case 4: //turn on port E bit 5 (5th led)
			PORTE &= (1<<PORTE6);
			break;
		default:
			break;
	}
	return;
}

int main(void)
{
	initialize_ports();
	light_simon_led(4);
	//delay()
	//PORTD = 0xFF;
	while(1);
	/*
	//TIMSK0 = 1;
	//MCUCSR0 = 1;
	//MCUCSR = 1<<ISC2;   // rising edge of external interrupt generates interrupt request
	//GICR = (1<<INT2);   // enable external interrupt on int2
	sei();				// set interrupt enable
	initialization_function();
	LCD_initializer();
	exit;
	
	/*
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