/* GOAL:
 * 1. Continuous countdown
 * 2. Play pause button (active-high)
 * 3. Keypad input for increment decrement when countdown is paused
 */
#include <xc.h> // include file for xc8 compiler

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

// for interrupt
int myINTF = 0;	  // we use for interrupt
int myTMR0IF = 0; // we use for delay overflow

// for keypad
int keypadFlag = 0;				 // keypad flag
unsigned char keypadData = 0x00; // keypad data in hex

// for counter logic
int joinedCounter = 0;
unsigned char counter1 = '2'; // counter 1 in lcd
unsigned char counter2 = '4'; // counter 2 in lcd
unsigned char currentCounter1;
unsigned char currentCounter2;

void delay(int count)
{
	int of_count = 0;

	while (of_count < count)
	{
		if (myTMR0IF)
		{
			of_count++;
			myTMR0IF = 0;
		}
	}
}

void interrupt ISR()
{
	GIE = 0;

	if (INTF)
	{
		INTF = 0;	 // reset interrupt flag manually
		myINTF ^= 1; // xor

		keypadFlag = 1;
		keypadData = PORTD & 0x0F; // mask data
	}

	if (TMR0IF) // Timer overflow interrupt
	{
		TMR0IF = 0; // reset timer flag manually
		myTMR0IF = 1;
	}

	GIE = 1;
}

void instCtrl(unsigned char data)
{
	PORTC = data; // load data to port c since my port c is the output pins
	RB5 = 0;	  // set RS to 0 (instruction reg)
	RB6 = 0;	  // set RW to 0 (write)
	RB7 = 1;	  // set E to 1
	delay(5);
	RB7 = 0; // set E to 0 (strobe)
}

void initLCD()
{
	delay(61);

	instCtrl(0x3C); // function set: 8-bit; dual-line
	instCtrl(0x38); // display off
	instCtrl(0x01); // display clear
	instCtrl(0x06); // entry mode: increment; shift off
	instCtrl(0x0E); // display on; cursor off; blink off
}

void dataCtrl(unsigned char data)
{
	PORTC = data; // load data to PORT C since its our output
	RB5 = 1;	  // set RS to 1 (data reg)
	RB6 = 0;	  // set RW to 0 (write)
	RB7 = 1;	  // set E to 1
	delay(5);
	RB7 = 0; // set E to 0 (strobe)
}

void clearDataResetCursor()
{
	instCtrl(0xC0);
}

void joinChar()
{
	// join chars to int
	unsigned char num1 = currentCounter1 - '0';
	unsigned char num2 = currentCounter2 - '0';

	joinedCounter = num1 * 10 + num2; // Convert to full number
}

void splitChar()
{
	// Split back to characters
	currentCounter1 = (joinedCounter / 10) + '0';
	currentCounter2 = (joinedCounter % 10) + '0';
}

void incrementCounter()
{
	// // Convert char digits to integers
	// joinChar();

	// if (joinedCounter == 24)
	// {
	// 	joinedCounter = 0; // Reset to zero if max reached
	// }
	// else
	// {
	// 	joinedCounter++; // Increment normally
	// }

	// splitChar();

	// update lcd
	// clearDataResetCursor();
	// dataCtrl(currentCounter1);
	// dataCtrl(currentCounter2);
}

void decrementCounter()
{
	// joinChar();

	// if (joinedCounter == 0)
	// {
	// 	joinedCounter = 24; // Wrap to max value
	// }
	// else
	// {
	// 	joinedCounter--; // Decrement normally
	// }

	// splitChar();

	// update lcd
	// clearDataResetCursor();
	// dataCtrl(currentCounter1);
	// dataCtrl(currentCounter2);
}

void processKeypadInput()
{
	delay(10);

	if (keypadFlag)
	{
		keypadFlag = 0; // Reset flag manually

		switch (keypadData)
		{
		case 0x0C:
			instCtrl(0xC0);
			dataCtrl('1');
			dataCtrl('1');
			dataCtrl('1');
			delay(10);
			break;
		case 0x0E:
			instCtrl(0xC0);
			dataCtrl('2');
			dataCtrl('2');
			dataCtrl('2');
			delay(10);
			break;
		default:
			instCtrl(0xC0);
			dataCtrl('4');
			dataCtrl('3');
			dataCtrl('3');
			delay(10);
			break;
		}

		// if (keypadData == 0x0C) // increment when * is pressed
		// {
		// 	incrementCounter();
		// }
		// else if (keypadData == 0x0E) // decrement when # is pressed
		// {
		// 	decrementCounter();
		// }
		// else
		// {
		// 	// do nothing
		// }
	}
}

void main()
{
	// io config
	TRISC = 0x00; // all ports C are output to LCD
	TRISB = 0x01; // Set PORTB as output
	TRISD = 0xFF; // all ports D are input from IC

	// timer overflow config
	OPTION_REG = 0xC4;
	INTE = 1;
	INTF = 0;
	TMR0IE = 1;
	GIE = 1;

	initLCD();
	instCtrl(0x02);

	dataCtrl('T');
	dataCtrl('I');
	dataCtrl('M');
	dataCtrl('E');
	dataCtrl(':');
	instCtrl(0xC0); // Move cursor

	currentCounter1 = counter1;
	currentCounter2 = counter2;

	while (1)
	{
		if (!myINTF)
		{
			if (RD4 == 1)
			{
				processKeypadInput();
			}
		}
		else
		{
			dataCtrl(currentCounter1);
			dataCtrl(currentCounter2);

			if (currentCounter2 == '0')
			{
				if (currentCounter1 != '0')
				{
					currentCounter1--;
					currentCounter2 = '9';
				}
				else
				{
					currentCounter1 = counter1;
					currentCounter2 = counter2;
				}
			}
			else
			{
				currentCounter2--;
			}

			clearDataResetCursor(); // move cursor and overwrite
			delay(25);
		}
	}
}
