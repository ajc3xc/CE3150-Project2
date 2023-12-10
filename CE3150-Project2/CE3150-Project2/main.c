/*
	Comp Eng 3150 Project 2
	Members: Adam Camerer, Evan Parrish, Ethan Mollet, Jerrett Martin
	Date: 12/5/2023
*/

// using 8mhz to make time delays easier
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <time.h>

//8 levels seems reasonable for a demo
//theoretically, 15 levels is possible with the middle right led
//but I need to demonstrate what happens when the timer overflows for the video
#define MAX_LEVEL 8

//used in half second delay function
#define HALF_SECOND_TIMER_VAL 0
#define HALF_SECOND_ITERATIONS 15

//used in minute time limit function
#define FOUR_SECOND_TIME_VAL -31250
#define TIMER_1_ITERATIONS 15
int timer_1_iterations_counter = 0;
int timed_out = 0;

//used for playing the game
int SEQUENCE[MAX_LEVEL];
int LVL = 1;

//////////////////////////////////////////////////////////////////////////
//						Initialization Functions						//

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

//						Initialization Functions						//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//						 Time Delay Functions							//
//1 minute time delay
//used when playing the simon game
void start_minute_time_limit() {
	timed_out = 0; //needs to be reset here in case you won and the timer is still running
	timer_1_iterations_counter = 0;
	TCNT1 = FOUR_SECOND_TIME_VAL;
	TCCR1A = 0x00; //normal clock
	TCCR1B |= 0x05; // normal clock, prescaler 1024
	TIMSK1 |= (1 << TOIE1);
	sei();
}

// called every 4 seconds
ISR(TIMER1_OVF_vect) {
	if (timer_1_iterations_counter < TIMER_1_ITERATIONS) {
		TCNT1 = FOUR_SECOND_TIME_VAL;
		timer_1_iterations_counter++;	
	}
	else
	{
		TIMSK1 &= ~(1 << TOIE1);
		TCNT1 = 0;
		timer_1_iterations_counter = 0;
		timed_out = 1;
	}
}


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

//used by play_sequence function
//returns 2 ^ (power_to + 1)
//must be 1, 2, 3, 4, or it returns 0
int two_to_the_power_of(int power_to)
{
	if ((power_to <= 0) || (power_to > 4)) return 0;
	
	int return_val = 1;
	for(int j=0; j <= power_to; j++)
	{
		return_val *= 2;
	}
	
	return return_val;
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
		PORTE ^= (1<<PORTE4); //toggle PORTE4
		TCNT2 = TIMER_VAL; //reset tcnt2
		TIFR2 = 1<<TOV2; //need to reset the overflow flag bit
		counter++;
	}
	
	//turn off timer0
	TCCR2A = 0x00;
	TCCR2B = 0x00;
	TIFR2 = 1<<TOV2;
}

//	Time Delay Functions (basically any function utilizing the timers)	//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//					Game End Functions									//

//turn off all the lights
void turn_off_leds()
{
	PORTD = 0xFF;
	PORTE |= (1<<PORTE5);
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

//					Game End Functions									//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//					Game Start Functions								//

void generate_simon_pattern(){
	//since this can be called multiple times, srand shouldn't be here (it regenerates the same pattern)
	//it should be in main
	for(int i=0 ; i<MAX_LEVEL ; i++){
		SEQUENCE[i] = (rand() % 4) + 1; //generate a random number from 1-4 inclusive
	}
}

//initial / restart 'screen'
//press button 1 to escape it to play the game
void wait_until_ready_to_play()
{
	int waiting = 1;
	while (waiting)
	{
		half_second_delay();
		half_second_delay();
		if (!(PINA & (1 << PINA0)))
		{
			waiting = 0;
			//keep light on and play sound while button is held down
			PORTD &= ~(1<<PORTD0);
			play_speaker(1);
			while (!(PINA & (1 << PINA0))); //wait until unclicked
		}
		PORTD ^= (1<<PORTD0);
	}
	PORTD |= (1<<PORTD0);
}

//					Game Start Functions								//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//				Game Playing Functions									//

//choose which led to light up
//used when selecting a button
void light_simon_led(int led_to_light)
{
	//0 means on, 1 means off	
	//turn off simon specific leds
	PORTD |= (1<<PORTD7);
	PORTD |= (1<<PORTD6);
	PORTD |= (1<<PORTD4);
	PORTE |= (1<<PORTE5);
	
	switch(led_to_light)
	{
		case 1: //turn on port D bit 7 (9th led)
			PORTE &= ~(1<<PORTE5);
			break;
		case 2: //turn on port D bit 1 (8th led)
			PORTD &= ~(1<<PORTD4);
			break;
		case 3: //turn on port D bit 4 (6th led)
			PORTD &= ~(1<<PORTD6);
			break;
		case 4: //turn on port E bit 5 (4th led)
			PORTD &= ~(1<<PORTD7);
			break;
		default:
			break;
	}
	return;
}

//display the current level on D1-D4 as 3 bit value
void display_level_leds()
{
	//turn off leds 1-3
	PORTD |= (1<<PORTD0);
	PORTD |= (1<<PORTD1);
	PORTD |= (1<<PORTD2);
	PORTD |= (1<<PORTD3);
	
	//this logic only works if there is up to 8 levels, and there isn't a level 0
	//as there is only 3 leds to display the level
	if (LVL==0 || LVL > MAX_LEVEL)	return;
	//display current levels on leds
	PORTD &= ~(LVL); //(used first 4 leds (LED1-4))
	
}

//plays the sequence of leds you need to get right
//sequence of (1, 2, 3, 4) values
void play_sequence()
{
	for(int i=0; i<LVL; i++){
		light_simon_led(SEQUENCE[i]);
		play_speaker(two_to_the_power_of(SEQUENCE[i]));
		light_simon_led(0);
		half_second_delay();
	}
}

//wait until a button clicked, then return the value
int wait_until_button_clicked()
{
	int button_state = 0;
	while(button_state==0)
	{
		//all these buttons wait until they're unpressed before continuing
		//prevents unintentional button spamming
		//the led is lit and the buzzer is played, and then waits until the button is released
		//button_states 1, 2, 3, 4 = buttons 5, 6, 8, 9
		//button_state 5 means the time limit was reached
		
		//button 5 pressed
		if (!(PINE & (1 << PINE6)))
		{
			button_state = 1;
			light_simon_led(button_state);
			play_speaker(two_to_the_power_of(button_state));
			while (!(PINE & (1 << PINE6)));
		}
		//button 6 pressed
		else if (!(PINA & (1 << PINA4)))
		{
			button_state = 2;
			light_simon_led(button_state);
			play_speaker(two_to_the_power_of(button_state));
			while (!(PINA & (1 << PINA4)));
		}
		//button 8 pressed
		else if (!(PINA & (1 << PINA6)))
		{
			button_state = 3;
			light_simon_led(button_state);
			play_speaker(two_to_the_power_of(button_state));
			while (!(PINA & (1 << PINA6)));
		}
		//button 9 pressed
		else if (!(PINA & (1 << PINA7)))
		{
			button_state = 4;
			light_simon_led(button_state);
			play_speaker(two_to_the_power_of(button_state));
			while (!(PINA & (1 << PINA7)));
		}
		else if((timed_out))
		{
			button_state = 5; //5 will always be invalid
			//timed_out = 0; //reset timed_out variable
		}
		light_simon_led(0); //turn off all led lights
	}
	return button_state;
}

void play_game()
{
	int button_state = 0;
	
	//regenerates the list of led patterns
	generate_simon_pattern();
	LVL = 1;
	
	int playing = 1;
	while(playing)
	{
		if(LVL > MAX_LEVEL)
		{
			playing = 0;
			win_game();
		}
		else
		{
			display_level_leds();
			play_sequence();
			int buttons_pressed = 0;
			start_minute_time_limit(); //minute time delay starts each level
			while((buttons_pressed < LVL) && playing)
			{
				int button_clicked = wait_until_button_clicked();
				if (button_clicked == SEQUENCE[buttons_pressed]) buttons_pressed++;
				else
				{
					lose_game();
					playing = 0;
				}
			}
			if (playing) LVL++;
			
			//wait a second until moving to the next level
			//that way the lights and sounds don't overlay
			half_second_delay();
			half_second_delay();
		}
	}
}

//				Game Playing Functions									//
//////////////////////////////////////////////////////////////////////////

int main(void)
{
	initialize_ports(); //sets which ports to use as i/o, turns off all the leds
	srand(time(NULL)); //set random number seed (it is always the same initial seed, but I just don't care)
	while(1)
	{
		//display blinking red led
		wait_until_ready_to_play();
		//wait a second after button pressed
		half_second_delay();
		half_second_delay();
		//play game
		play_game();
	}

	return 0;
}