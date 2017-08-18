/* Joseph Pacia - jpaci001@ucr.edu
 * Lab Section 023
 * Final Project: Hide-and-Seek Robocar
 * 
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "timer.h"
#include "bit.h"
#include "io.c"
#include "scheduler.h"
#include "pwm.c"

//#define GSIZE 40
#define VSIZE 28
#define DSIZE 17
//#define VDSIZE 10
//#define DDSIZE 4
//#define LSIZE 13

//---------InGame Music Sequence----------------
//double gNotes[GSIZE] = {261.63, 523.25, 220.00, 440.00, 233.08, 466.16, 0, 0, 0, 0, 261.63, 523.25, 220.00, 440.00, 233.08, 466.16, 0, 0, 0, 0,
//				 174.61, 349.23, 146.83, 293.66, 155.56, 311.13, 0, 0, 0, 0, 174.61, 349.23, 146.83, 293.66, 155.56, 311.13, 0, 0, 0, 0};
//---------------------------------------------


//------------Victory Music Sequence-------------
double vNotes[VSIZE] =				{196.00, 261.63, 329.63, 392.00, 523.25, 659.25, 783.99, 659.25, 207.65, 261.63, 311.13, 415.30, 523.25, 622.25, 830.61, 622.25,
								233.08, 293.66, 349.23, 466.16, 587.33, 698.46, 932.33, 932.33, 932.33, 932.33, 1046.50, 0   };
unsigned char vNoteLengths[VSIZE] =	{0x01,   0x01,   0x01,   0x01,   0x01,   0x01,   0x03,   0x03,   0x01,   0x01,   0x01,   0x01,   0x01,   0x01,   0x03,   0x03,
								0x01,   0x01,   0x01,   0x01,   0x01,   0x01,   0x03,   0x01,   0x01,   0x01,   0x06,    0x06};
//-----------------------------------------------


//--------------Defeat Music Sequence-------------
double dNotes[DSIZE] =				{523.25, 0,    0,    392.00, 0,    0,    329.63, 440.00, 493.88, 440.00, 415.30, 466.16, 415.30, 392.00, 349.23, 392.00, 0   };
unsigned char dNoteLengths[DSIZE] =	{0x01,   0x01, 0x01, 0x01,   0x01, 0x01, 0x02,   0x02,   0x02,   0x02,   0x02,   0x02,   0x02,   0x01,   0x01,   0x04,   0x06};
//-----------------------------------------------


//---------------Victory Dance Sequence---------------
//unsigned char v_dance_seq[VDSIZE] = {0x05, 0x0A, 0x05, 0x0A, 0x09, 0x0A, 0x05, 0x0A, 0x05, 0x06};
//unsigned char v_dance_len[VDSIZE] = {0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x01, 0x01, 0x01, 0x04};
//----------------------------------------------------


//----------------Defeat Dance Sequence---------------
//unsigned char d_dance_seq[DDSIZE] = {0x06, 0x09, 0x06, 0x09};
//-----------------------------------------------------


//----------------LED Sequence----------------------
//unsigned char ledShow[LSIZE] =   {0x01, 0x02, 0x04, 0x02, 0x01, 0x02, 0x04, 0x02, 0x03, 0x06, 0x05, 0x02, 0x07};
//unsigned char ledLength[LSIZE] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04};
//--------------------------------------------------


//--------------SM Trigger Variables-----------------
unsigned char asleep;
unsigned char awake;
unsigned char start;
unsigned char rove;
unsigned char victory;
unsigned char defeat;
//---------------------------------------------------


//---------Other Shared Variables--------------------
unsigned char motionSens;
unsigned char leftSens;
unsigned char frontSens;
unsigned char rightSens;
unsigned char timeLimit;
unsigned char cntdwnTime;
//unsigned char motors;
unsigned char UT_count;
unsigned char right;
unsigned char left;
unsigned char lcdCnt;
unsigned char LEDs;
unsigned char gLengthCnt;
unsigned char vLengthCnt;
unsigned char dLengthCnt;
unsigned char tenCnt;
unsigned char vdLengthCnt;
unsigned char ddLengthCnt;
unsigned char ledLengthCnt;
unsigned char lcdChar;
unsigned char roveCount;
unsigned char scanCount;
unsigned char rFound;
unsigned char motionCount;
unsigned char scanDur;
int lcdI;
int gIndex;
int vIndex;
int dIndex;
int vDanceI;
int dDanceI;
int ledIndex;
//---------------------------------------------

//-----------Sleep/Wake SM--------------------
enum SW_States{buttonInit, Asleep, Waking, Awake, PuttoSleep, Starting, Started, Reset};

int SW_Tick(int state)
{
	unsigned char sw_button = ~PINA & 0x01;
	unsigned char s_button = ~PINA & 0x02;

	switch (state) // transitions
	{
		case buttonInit:
			state = Asleep;
			break;
		case Asleep:
			if (sw_button)
				state = Waking;
			else
				state = Asleep;
			break;
		case Waking:
			if (!sw_button)
				state = Awake;
			else if (sw_button && s_button)
				state = Asleep;
			else
				state = Waking;
			break;
		case Awake:
			if (s_button)
				state = Starting;
			else if (sw_button)
				state = PuttoSleep;
			else
				state = Awake;
			break;	
		case PuttoSleep:
			if (!sw_button)
				state = Asleep;
			else if (sw_button && s_button)
				state = Awake;
			else
				state = PuttoSleep;
			break;
		case Starting:
			if (!s_button)
			{
				start = 0x01;
				state = Started;
			}
			else if (s_button && sw_button)
				state = Awake;
			else
				state = Starting;
			break;
		case Started:
			if (start == 0x00 || rove == 0x00)
				state = Awake;
			else if (s_button)
			{
				start = 0x00;
				state = Reset;
			}
			else
				state = Started;
			break;
		case Reset:
			if (!s_button)
				state = Awake;
			else
				state = Reset;
			break; // missed one here
		default:
			state = buttonInit;
			break;
	}
	switch (state) // state actions
	{
		case buttonInit:
			start = 0x00;
			break;
		case Asleep:
			//PORTD = 0x00;
			asleep = 0x01;
			awake = 0x00;
			break;
		case Waking:
			break;
		case Awake:
			//PORTD = 0x02;
			awake = 0x01;
			asleep = 0x00;
			break;
		case PuttoSleep:
			break;
		case Starting:
			break;
		case Started:
			break;
		case Reset:
			LCD_DisplayString(0, "Release to reset.");
			break;
		default:
			break;
	}
	return state;
}
//----------------------------


//------------Game SM-----------------
enum G_States{g_init, g_wait, cntdwn, ready, inProg, found, notFound};

int G_Tick(int state)
{
	//motionSens = ~PINA & 0x04;

	switch (state) // transitions
	{
		case g_init:
			state = g_wait;
			break;
		case g_wait:
			if (start == 0x01)
				state = cntdwn;
			else
				state = g_wait;
			break;
		case cntdwn:
			if (cntdwnTime <= 0x00)
			{
				cntdwnTime = 0x00;
				state = ready;
			}
			else if (start == 0x00)
				state = g_wait;
			else
			{
				cntdwnTime = cntdwnTime - 0x01;
				state = cntdwn;
			}
			break;
		case ready:
			if (cntdwnTime >= 0x08)
			{
				cntdwnTime = 0x00;
				state = inProg;
			}
			else
			{
				cntdwnTime = cntdwnTime + 0x01;
				state = ready;
			}
			break;
		case inProg:
			//if (timeLimit <= 0x28)
			//{
				//if (tenCnt >= 0x00 && tenCnt < 0x02)
				//{
					//set_PWM(220.00);
					//tenCnt = tenCnt + 0x01;
				//}
				//else if (tenCnt >= 0x02 && tenCnt < 0x04)
				//{
					//set_PWM(0);
					//tenCnt = tenCnt + 0x01;
				//}
				//else tenCnt = 0x00;
			//}
			if (start == 0x00)
				state = g_wait;
			if (timeLimit > 0x00) // continue
			{
				//if (motionSens)
				//{
					//tenCnt = 0x00;
					//state = found;
				//}
				if (rFound == 0x01)
					state = found;
			}
			else if (timeLimit <= 0x00)
			{
				//tenCnt = 0x00;
				state = notFound;
			}
			else
			{
				timeLimit = timeLimit - 0x01;
				state = inProg;
			}
			break;
		case found:
			if (start == 0x00 || victory == 0x00 || rFound == 0x00) 
				state = g_wait;
			else 
				state = found;
			break;
		case notFound:
			if (start == 0x00 || defeat == 0x00) 
				state = g_wait;
			else 
				state = notFound;
			break;
		default:
			state = g_init;
			break;
	}
	switch (state) // state actions
	{
		case g_init:
			break;
		case g_wait:
			timeLimit = 0x78;
			cntdwnTime = 0x44;
			victory = 0x00;
			defeat = 0x00;
			rove = 0x00;
			rFound = 0x00;
			break;
		case cntdwn:
			break;
		case inProg:
			rove = 0x01;
			break;
		case found:
			victory = 0x01;
			rove = 0x00;
			break;
		case notFound:
			defeat = 0x01;
			rove = 0x00;
			break;
		default:
			break;
	}
	return state;
}
//------------------------------------


//---------------Movement SM--------------
enum moveStates{initMove, m_wait, straight, TL, TR, RL, RR, UT, reverse, scan, rScan};

int moveTick(int state) // continue
{
	motionSens = ~PINA & 0x04;
	leftSens = ~PINA & 0x08;
	frontSens = ~PINA & 0x10;
	rightSens = ~PINA & 0x20;
	switch(state) // transitions
	{
		case initMove:
			state = m_wait;
			break;
		case m_wait:
			if (rove == 0x01 && start == 0x01) 
			{
				PORTB = 0x05;
				state = straight;
			}
			else 
				state = m_wait;
			break;
		case straight:
			if (rove == 0x00 || start == 0x00)
				state = m_wait;
			if (roveCount >= 0x28)
			{
				roveCount = 0x00;
				state = scan;
			}
			else
			{
				roveCount = roveCount + 0x01;
				if (rightSens)
				{
					if (frontSens)
						state = RL;
					else
						state = TL;
				}
				else if (leftSens)
				{
					if (frontSens)
						state = RR;
					else
						state = TR;
				}
				else if (frontSens && !leftSens && !rightSens)
					state = reverse;
				else if (leftSens && frontSens && rightSens)
					state = UT;
				else
					state = straight;
			}
			break;
		case TL:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			//if (roveCount >= 0x28)
			//{
				//roveCount = 0x00;
				//state = scan;
			//}
			//else
			//{
				//roveCount = roveCount + 0x01;
				//if (rightSens)
				//state = TL;
				//else if (rove == 0x00)
				//state = m_wait;
				//else
				state = straight;
			//}
			break;
		case RL:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			//if (roveCount >= 0x28)
			//{
				//roveCount = 0x00;
				//state = scan;
			//}
			//else
			//{
				//roveCount = roveCount + 0x01;
				if (left >= 0x03)
				{
					left = 0x00;
					state = RR;
				}
				//if (rightSens)
				//{
					//if (frontSens)
					//state = RL;
					//else
					//state = TL;
				//}
				else
				{
					left = left + 0x01;
					state = straight;
				}
			//}
			break;
		case UT:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			//if (roveCount >= 0x28)
			//{
				//roveCount = 0x00;
				//state = scan;
			//}
			//else
			//{
				//roveCount = roveCount + 0x01;
				if (UT_count >= 0x04)
				{
					UT_count = 0x00;
					state = straight;
				}
				else
				{
					UT_count = UT_count + 0x01;
					state = UT;
				}
			//}
			break;
		case RR:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			//if (roveCount >= 0x28)
			//{
				//roveCount = 0x00;
				//state = scan;
			//}
			//else
			//{
				//roveCount = roveCount + 0x01;
				if (right >= 0x03)
				{
					right = 0x00;
					state = RL;
				}
				//if (leftSens)
				//{
					//if (frontSens) 
						//state = RR;
					//else 
						//state = TR;
				//}
				else 
				{
					right = right + 0x01;
					state = straight;
				}
			//}
			break;
		case TR:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			//if (roveCount >= 0x28)
			//{
				//roveCount = 0x00;
				//state = scan;
			//}
			//else
			//{
				//roveCount = roveCount + 0x01;
				//if (leftSens) 
					//state = TR;
				//else 
					state = straight;
			//}
			break;
		case reverse:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			//if (roveCount >= 0x28)
			//{
				//roveCount = 0x00;
				//state = scan;
			//}
			//else
			//{
				//roveCount = roveCount + 0x01;
				if (rove == 0x01 && start == 0x01)
					state = RR;
			//}
			break;
		case scan:
			if (motionSens)
			{
				if (motionCount >= 0x0F)
				{
					motionCount = 0x00;
					rFound = 0x01;
					state = m_wait;
				}
				else
					motionCount = motionCount + 0x01;
			}
			if (scanDur >= 0x02)
			{
				scanDur = 0x00;
				if (scanCount < 0x04)
				{
					scanCount = scanCount + 0x01;
					state = rScan;
				}
				else
				{
					scanCount = 0x00;
					state = straight;
				}
			}
			else
			{
				scanDur = scanDur + 0x01;
				state = scan;
			}
			break;
		case rScan:
			//if (rove == 0x00 || start == 0x00)
				//state = m_wait;
			if (scanDur >= 0x03)
			{
				scanDur = 0x00;
				state = scan;
			}
			else
			{
				scanDur = scanDur + 0x01;
				state = rScan;
			}
			break;
		default:
			state = initMove;
			break;						
	}
	switch(state) // state actions
	{
		case initMove:
			break;
		case m_wait:
			PORTB = 0x00;
			scanCount = 0x00;
			motionCount = 0x00;
			scanDur = 0x00;
			break;
		case straight:
			//PORTD = 0x02;
			PORTB = 0x05;
			break;
		case TL:
			//PORTD = 0x06;
			PORTB = 0x01;
			break;
		case RL:
			//PORTD = 0x06;
			PORTB = 0x09;
			break;
		case UT:
			//PORTD = 0x03;
			PORTB = 0x06;
			break;
		case RR:
			//PORTD = 0x03;
			PORTB = 0x06;
			break;
		case TR:
			//PORTD = 0x03;
			PORTB = 0x04;
			break;
		case reverse:
			//PORTD = 0x07;
			PORTB = 0x0A;
			break;
		case scan:
			PORTB = 0x00;
			//PORTD = 0x02;
			break;
		case rScan:
			PORTB = 0x06;
			//PORTD = 0x03;
			break;
		default:
			break;
	}
	return state;
}
//--------------------------------------


//----------------LCD SM----------------
enum LCDStates{lcdInit, lcdSleep, lcdAwake, lcdCntdwn, lcdReady, lcdGame, lcdFound, lcdNotFound};

int LCD_Tick(int state)
{
	switch (state) // transitions
	{
		case lcdInit:
			state = lcdSleep;
			break;
		case lcdSleep:
			if (awake == 0x01) 
				state = lcdAwake;
			else 
				state = lcdSleep;
			break;
		case lcdAwake:
			if (asleep == 0x01) 
				state = lcdSleep;
			else if (start == 0x01) 
			{
				lcdI = 10;
				state = lcdCntdwn;
			}
			else 
				state = lcdAwake;
			break;
		case lcdCntdwn:
			if (start == 0x00)
				state = lcdAwake;
			if (cntdwnTime > 0x00 && lcdCnt < 0x04)
			{
				if (lcdCnt >= 0x00 && lcdCnt < 0x02)
				{
					//PORTD = 0x20;
					set_PWM(220.00);
				}
				else if (lcdCnt >= 0x02 && lcdCnt < 0x04)
				{
					//PORTD = 0x00;
					set_PWM(0);
				}
			}
			if (lcdCnt >= 0x04 && cntdwnTime > 0x00)
			{
				lcdCnt = 0x00;
				//lcdI = lcdI - 1;
			}
			else if (cntdwnTime <= 0x00)
			{
				lcdCnt = 0x00;
				//lcdI = 60;
				state = lcdReady;
			}
			else
			{
				lcdCnt = lcdCnt + 0x01;
				state = lcdCntdwn;
			}
			break;
		case lcdReady:
			if (start == 0x00)
				state = lcdAwake;
			if (cntdwnTime >= 0x08 || rove == 0x01)
			{
				cntdwnTime = 0x00;
				state = lcdGame;
			}
			else
			{
				//PORTD = 0x02;
				if (cntdwnTime >= 0x00 && cntdwnTime < 0x04)
				{
					set_PWM(440.00);
					LCD_DisplayString(1, "Ready or not");
				}
				else if (cntdwnTime >= 0x04 && cntdwnTime < 0x08)
				{
					set_PWM(0);
					LCD_DisplayString(1, "Here I come!");
				}
				//cntdwnTime = cntdwnTime + 0x01;
				state = lcdReady;
			}
			break; // 2nd miss
		case lcdGame:
			if (start == 0x00)
				state = lcdAwake;
			if (victory == 0x01)
			{
				lcdCnt = 0x00;
				state = lcdFound;
			}
			else if (defeat == 0x01)
			{
				lcdCnt = 0x00;
				state = notFound;
			}
			else
			{
				//if (lcdCnt >= 0x04)
				//{
					//lcdCnt = 0x00;
					//lcdI = lcdI - 1;
				//}
				//else lcdCnt = lcdCnt + 0x01;
				state = lcdGame;
			}
			break;
		case lcdFound:
			if (victory == 0x00) 
				state = lcdAwake;
			else 
				state = lcdFound;
			break;
		case lcdNotFound:
			if (defeat == 0x00) 
				state = lcdAwake;
			else 
				state = lcdNotFound;
			break;
		default:
			state = lcdInit;
			break;
	}
	switch (state) // state actions
	{
		case lcdInit:
			break;
		case lcdSleep:
			LCD_DisplayString(1, "ZZZZZ...");
			break;
		case lcdAwake:
			LCD_DisplayString(1, "Hello! Let's    play!");
			break;
		case lcdCntdwn:
			LCD_DisplayString(1, "Starting in: ");
			LCD_WriteData(lcdI + '0');
			break;
		case lcdReady:
			break;
		case lcdGame:
			LCD_DisplayString(1, "Where are you??");
			//LCD_WriteData(lcdI + '0');
			break;
		case lcdFound:
			LCD_DisplayString(1, "I found you!!!  :D");
			break;
		case lcdNotFound:
			LCD_DisplayString(1, "Couldn't find   you :(");
			break;
		default:
			break;
	}
	return state;
}
//------------------------------------


//-----------------InGame Music SM-----------------
//enum gMusicStates{gMusicInit, gMusicWait, play};
//
//int musicTick(int state)
//{
	//switch(state) // transitions
	//{
		//case gMusicInit:
			//state = gMusicWait;
			//break;
		//case gMusicWait:
			//if (rove == 0x01)
				//state = play;
			//else
				//state = gMusicWait;
			//break;
		//case play:
			//if (gLengthCnt >= 0x01)
			//{
				//gLengthCnt = 0x00;
				//gIndex = gIndex + 1;
			//}
			//else
				//gLengthCnt = gLengthCnt + 0x01;
			//if (gIndex >= GSIZE)
				//gIndex = 0;
			//if ((victory == 0x01 || defeat == 0x01) || rove == 0x00)
				//state = gMusicWait;
			//else
				//state = play;
			//break;
		//default:
			//state = gMusicInit;
			//break;
	//}
	//switch(state) // state actions
	//{
		//case gMusicInit:
			//break;
		//case gMusicWait:
			//break;
		//case play:
			//set_PWM(gNotes[gIndex]);
			//break;
		//default:
			//break;
	//}	
	//return state;
//}

//----------------------------------------------


//-------------Victory Music SM---------------------
enum vMusicStates{vMusicInit, vMusicWait, playVictory};

int victoryTick(int state)
{
	switch(state) // transitions
	{
		case vMusicInit:
			state = vMusicWait;
			break;
		case vMusicWait:
			if (victory == 0x01 && rove == 0x00)
				state = playVictory;
			else
				state = vMusicWait;
			break;
		case playVictory:
			if (vLengthCnt >= vNoteLengths[vIndex])
			{
				vLengthCnt = 0x00;
				vIndex = vIndex + 1;
			}
			else
				vLengthCnt = vLengthCnt + 0x01;
			if (vIndex >= VSIZE)
			{
				vIndex = 0;
				set_PWM(0);
				start = 0x00;
				victory = 0x00;
				rFound = 0x00;
				state = vMusicWait;
			}
			else
				state = playVictory;
			break;
		default:
			state = vMusicInit;
			break;
	}
	switch(state) // state actions
	{
		case vMusicInit:
			break;
		case vMusicWait:
			break;
		case playVictory:
			set_PWM(vNotes[vIndex]);
			break;
		default:
			break;
	}
	return state;
}
//-----------------------------------------


//----------------Defeat Music SM---------------
enum dMusicStates{dMusicInit, dMusicWait, playDefeat};

int defeatTick(int state)
{
	switch(state) // transitions
	{
		case dMusicInit:
			state = dMusicWait;
			break;
		case dMusicWait:
			if (defeat == 0x01 && rove == 0x00)
				state = playDefeat;
			else
				state = dMusicWait;
			break;
		case playDefeat:
			if (dLengthCnt >= dNoteLengths[dIndex])
			{
				dLengthCnt = 0x00;
				dIndex = dIndex + 1;
			}
			else
				dLengthCnt = dLengthCnt + 0x01;
			if (dIndex >= DSIZE)
			{
				dIndex = 0;
				set_PWM(0);
				start = 0x00;
				defeat = 0x00;
				state = dMusicWait;
			}
			else
				state = playDefeat;
			break;
		default:
			state = dMusicInit;
			break;
	}
	switch(state) // state actions
	{
		case dMusicInit:
			break;
		case dMusicWait:
			break;
		case playDefeat:
			set_PWM(dNotes[dIndex]);
			break;
		default:
			break;
	}
	return state;
}
//---------------------------------------------


//--------------Dance SM----------------------
//enum danceStates{danceInit, danceWait, vDance, dDance};
//
//int danceTick(int state)
//{
	//switch(state) // Transitions
	//{
		//case danceInit:
			//state = danceWait;
			//break;
		//case danceWait:
			//if (rove == 0x00)
			//{
				//if (victory == 0x01 && defeat == 0x00)
					//state = vDance;
				//else if (defeat == 0x01 && victory == 0x00)
					//state = dDance;
			//}
			//else 
				//state = danceWait;
			//break; // 3rd miss
		//case vDance:
			//if (vdLengthCnt >= v_dance_len[vDanceI])
			//{
				//vdLengthCnt = 0x00;
				//vDanceI = vDanceI + 1;
			//}
			//else
				//vdLengthCnt = vdLengthCnt + 0x01;
			//if (ledLengthCnt >= ledLength[ledIndex])
			//{
				//ledLengthCnt = 0x00;
				//ledIndex = ledIndex + 1;
			//}
			//else
				//ledLengthCnt = ledLengthCnt + 0x01;
			//if (vDanceI >= VDSIZE && ledIndex >= LSIZE)
			//{
				//ledIndex = 0;
				//vDanceI = 0;
				//PORTD = 0x02;
				//PORTB = 0x00;
				//state = danceWait;
			//}
			//else
				//state = vDance;
			//break;
		//case dDance:
			//if (ddLengthCnt >= 0x01)
			//{
				//ddLengthCnt = 0x00;
				//dDanceI = dDanceI + 1;
			//}
			//else
				//ddLengthCnt = ddLengthCnt + 0x01;
			//if (dDanceI >= DDSIZE)
			//{
				//dDanceI = 0;
				//PORTB = 0x00;
				//state = danceWait;
			//}
			//else
				//state = dDance;
			//break;
		//default:
			//state = danceInit;
			//break;
	//}
	//switch(state) // State actions
	//{
		//case danceInit:
			//break;
		//case danceWait:
			//break;
		//case vDance:
			//PORTB = v_dance_seq[vDanceI];
			////PORTB = motors;
			//PORTD = ledShow[ledIndex];
			//break;
		//case dDance:
			//PORTB = d_dance_seq[dDanceI];
			////PORTB = motors;
			//PORTD = 0x02;
			//break;
		//default:
			//break;
	//}
	//return state;
//}
//-------------------------------------------


//-------------Combine SM--------------------
//enum combineStates{cInit, cWait, combine};
//
//int combineTick(int state)
//{
	//switch(state) // transitions
	//{
		//case cInit:
			//state = cWait;
			//break;
		//case cWait:
			//if (victory == 0x00 || defeat == 0x00) 
				//state = combine;
			//else 
				//state = cWait;
			//break;
		//case combine:
			//if (victory == 0x01 || defeat == 0x01) 
				//state = cWait;
			//else 
				//state = combine;
			//break;
		//default:
			//state = cInit;
			//break;
	//}
	//switch(state) // state actions
	//{
		//case cInit:
			//break;
		//case cWait:
			//break;
		//case combine:
			//PORTB = motors;
			//PORTD = LEDs;
			//break;
		//default:
			//break;
	//}
	//return state;
//}
//----------------------------------------


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	// Task periods
	unsigned long int SW_Tick_calc = 500;
	unsigned long int G_Tick_calc = 250;
	unsigned long int moveTick_calc = 250;
	unsigned long int LCDTick_calc = 250;
	//unsigned long int musicTick_calc = 74;
	unsigned long int victoryTick_calc = 68;
	unsigned long int defeatTick_calc = 74;
	//unsigned long int danceTick_calc = 204;
	//unsigned long int combineTick_calc = 500;

	// Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SW_Tick_calc, G_Tick_calc);
	tmpGCD = findGCD(tmpGCD, moveTick_calc);
	tmpGCD = findGCD(tmpGCD, LCDTick_calc);
	//tmpGCD = findGCD(tmpGCD, musicTick_calc);
	tmpGCD = findGCD(tmpGCD, victoryTick_calc);
	tmpGCD = findGCD(tmpGCD, defeatTick_calc);
	//tmpGCD = findGCD(tmpGCD, danceTick_calc);
	//tmpGCD = findGCD(tmpGCD, combineTick_calc);

	// GCD for all tasks or smallest time unit for tasks
	unsigned long int GCD = tmpGCD;

	// Recalculate GCD periods for scheduler
	unsigned long int SW_Tick_period = SW_Tick_calc/GCD;
	unsigned long int G_Tick_period = G_Tick_calc/GCD;
	unsigned long int moveTick_period = moveTick_calc/GCD;
	unsigned long int LCDTick_period = LCDTick_calc/GCD;
	//unsigned long int musicTick_period = musicTick_calc/GCD;
	unsigned long int victoryTick_period = victoryTick_calc/GCD;
	unsigned long int defeatTick_period = defeatTick_calc/GCD;
	//unsigned long int danceTick_period = danceTick_calc/GCD;
	//unsigned long int combineTick_period = combineTick_calc/GCD;

	// Declare array of tasks
	static task SW_task, G_task, movement_task, LCD_task/*, music_task*/, victory_task, defeat_task/*, dance_task/*, combine_task*/;
	task *tasks[] = {&SW_task, &G_task, &movement_task, &LCD_task/*, &music_task*/, &victory_task, &defeat_task/*, &dance_task/*, &combine_task*/};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Sleep-Wake Task
	SW_task.state = -1; // Initial state
	SW_task.period = SW_Tick_period; // Task period
	SW_task.elapsedTime = SW_Tick_period; // Task current elapsed time
	SW_task.TickFct = &SW_Tick; // Function pointer for tick

	// Game Task
	G_task.state = -1; // Initial state
	G_task.period = G_Tick_period; // Task period
	G_task.elapsedTime = G_Tick_period; // Task current elapsed time
	G_task.TickFct = &G_Tick; // Function pointer for tick

	// Movement Task
	movement_task.state = -1; // Initial state
	movement_task.period = moveTick_period; // Task period
	movement_task.elapsedTime = moveTick_period; // Task current elapsed time
	movement_task.TickFct = &moveTick; // Function pointer for tick

	// LCD Task
	LCD_task.state = -1; // Initial state
	LCD_task.period = LCDTick_period; // Task period
	LCD_task.elapsedTime = LCDTick_period; // Task current elapsed time
	LCD_task.TickFct = &LCD_Tick; // Function pointer for tick

	// In-game Music Task
	//music_task.state = -1; // Initial state
	//music_task.period = musicTick_period; // Task period
	//music_task.elapsedTime = musicTick_period; // Task current elapsed time
	//music_task.TickFct = &musicTick;

	// Victory task
	victory_task.state = -1; // Initial state
	victory_task.period = victoryTick_period; // Task period
	victory_task.elapsedTime = victoryTick_period; // Task current elapsed time
	victory_task.TickFct = &victoryTick; // Function pointer for tick

	// Defeat task
	defeat_task.state = -1; // Initial state
	defeat_task.period = defeatTick_period; // Task period
	defeat_task.elapsedTime = defeatTick_period; // Task current elapsed time
	defeat_task.TickFct = &defeatTick; // Function pointer for tick

	// Dance task
	//dance_task.state = -1; // Initial state
	//dance_task.period = danceTick_period; // Task period
	//dance_task.elapsedTime = danceTick_period; // Task current elapsed time
	//dance_task.TickFct = &danceTick; // Function pointer for tick 

	// Combine task
	//combine_task.state = -1; // Initial state
	//combine_task.period = combineTick_period; // Task period
	//combine_task.elapsedTime = combineTick_period; // Task current elapsed time
	//combine_task.TickFct = &combineTick; // Function pointer for tick

	// Set timer and turn it on
	TimerSet(GCD);
	TimerOn();

	unsigned short i; // Scheduler for-loop iterator

	LCD_init();
	PWM_on();
	LEDs = 0x00;
	start = 0x00;
	timeLimit = 0x78;
	cntdwnTime = 0x44;
	rove = 0x00;
	UT_count = 0x00;
	left = 0x00;
	right = 0x00;
	victory = 0x00;
	defeat = 0x00;
	lcdCnt = 0x00;
	gLengthCnt = 0x00;
	vLengthCnt = 0x00;
	dLengthCnt = 0x00;
	gIndex = 0;
	vIndex = 0;
	dIndex = 0;
	tenCnt = 0x00;
	vDanceI = 0;
	dDanceI = 0;
	vdLengthCnt = 0x00;
	ddLengthCnt = 0x00;
	ledLengthCnt = 0x00;
	ledIndex = 0;
	lcdChar = 0x00;
	roveCount = 0x00;
	rFound = 0x00;
	scanCount = 0x00;
	motionCount = 0x00;
	scanDur = 0x00;
    while (1) 
    {
		// Scheduler code
		for (i = 0; i < numTasks; i++)
		{
			// Task is ready to tick
			if (tasks[i]->elapsedTime == tasks[i]->period)
			{
				// Set next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset elapsed time for next tick
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }

	// Error: Program should not exit
	return 0;
}

