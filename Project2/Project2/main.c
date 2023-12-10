/*
	Comp Eng 3150 Project 2 Code Conversion 
	Members: Adam Camerer, Evan Parrish, Ethan Mollet, Jerrett Martin
	Date: 12/7/2023
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t COUNT = 0; // Where we store the current count. We used R16 in ASM

void QDELAY(); // A function for causing a small amount of delay
void SET_LEDS(); // A function to update LEDs
void CHIRP(); // Plays a chirp from the speaker

int main(void)
{
	DDRA = 0x00; // Port A in input mode
	PORTA = 0xFF; // Enable pull-ups on PA
	
	DDRD = 0xFF; // Port D in output mode
	PORTD = 0xFF; // turn off LEDS (active low)
	
	DDRE = 0b00110000; // Set input/output for Port E
	PORTE = 0xFF; // ready Port E
	
    while (1) 
    {
		
		if ((PINA & (1<<6)) == 0) // If button 8 is pressed
		{
			// Count up (equivalent to CHECK_UP in asm)
			
			if (COUNT >= 30)
			{
				COUNT = 0; // wrap around to bottom
				CHIRP(); // play tone on speaker
			}
			else
			{
				COUNT++; // increment COUNT
			}
			
			SET_LEDS(); // update LED states
			QDELAY(); // wait a bit
		}
		else if ((PINA & (1<<5)) == 0) // Else if button 7 is pressed
		{
			// Count down (equivalent to CHECK_DOWN in asm)
			
			if (COUNT <= 0)
			{
				COUNT = 30; // wrap around to top
				CHIRP(); // play tone on speaker
			}
			else
			{
				COUNT--; // decrement COUNT
			}
			
			SET_LEDS(); // update LED states
			QDELAY(); // wait a bit
		}
		else if ((PINA & (1<<4)) == 0) // Else if button 6 is pressed
		{
			// Reset Count (equivalent to RESET_COUNT in asm)
			COUNT = 0;
			
			SET_LEDS(); // update LED states
			QDELAY(); // wait a bit
		}
    }
	
	return 0;
}

void QDELAY()
{
	_delay_ms(837.293); // like "CALL QDELAY" in asm
}

void SET_LEDS()
{
	PORTD = COUNT ^ 0b11111111; // Set LEDs according to one's complement of COUNT (since LEDs are active low) (SET_LEDS in asm)	
}

void CHIRP()
{
	for (uint8_t i = 0x2F; i > 0; i--)
	{
		PORTE &= ~(1<<4); // Set buzzer to high
		_delay_ms(12.788); // like SM_DELAY
		PORTE |= 1<<4; // Set buzzer to low
		_delay_ms(12.788); // like SM_DELAY
	}
}