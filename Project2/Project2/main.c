/*
 * Project2.c
 *
 * Created: 12/9/2023 6:35:52 AM
 * Author : Jerrett
 */ 

#include <avr/io.h>
#include <stdio.h>

void CHECK_UP();
void CHECK_DOWN();
void CHECK_RESET();
void CHECK_SHIFT_MODE();
void RESET_COUNT();

uint8_t COUNT = 0; // Where we store the current count. We used R16 in ASM

int main(void)
{
	DDRA = 0x00; // Port A in input mode
	PORTA = 0xFF; // Enable pull-ups on PA
	
	COUNT = 0;
	
    while (1) 
    {
		
		if ((PINA & (1<<6)) != 0) // Check if button 8 is pressed
		{
			if ((PINA & (1<<5)) != 0) // Check if button 7 is pressed
			{
				if ((PINA & (1<<4)) != 0) // Check if button 6 is pressed
				{
					CHECK_SHIFT_MODE();
				}
				else
				{
					RESET_COUNT();
				}
			}
			else
			{
				CHECK_DOWN();
			}
		}
		else
		{
			CHECK_UP();	
		}
		
		// TODO: Handle LEDS
		// TODO: Delay here
    }
	
	return 0;
}

void CHECK_UP()
{
	// Guard clause to check if we need to wrap around to bottom
	if (COUNT >= 30)
	{
		COUNT = 0; // wrap to bottom
		// TODO: TURN_ON_SPEAKER // play a tone
		return;
	}
	
	// otherwise, just increment COUNT
	++COUNT;
	return;
}

void CHECK_DOWN()
{
	// Guard clause to check if we need to wrap around to top
	if (COUNT <= 0)
	{
		COUNT = 30; // wrap to top
		// TODO: TURN_ON_SPEAKER // play a tone
		return;
	}
	
	// otherwise, just decrement COUNT
	--COUNT;
	return;
}

void RESET_COUNT()
{
	COUNT = 0;
	return;
}

void CHECK_SHIFT_MODE()
{
	printf("CHECK_SHIFT_MODE");
}
