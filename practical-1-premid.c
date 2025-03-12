#include <xc.h> // include file for xc8 compiler

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

unsigned char myINTF = 0;	// we use for interrupt
unsigned char myTMR0IF = 0; // we use for delay overflow

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
	delay(61);
}

void main()
{
	// io config
	TRISC = 0x00; // all ports C are output to LCD
	TRISB = 0x00; // Set PORTB as output
	TRISD = 0xFF; // all ports D are input from IC

	// timer overflow config
	OPTION_REG = 0xC4;
	INTE = 1;
	INTF = 0;
	TMR0IE = 1;
	GIE = 1;

	initLCD();
	instCtrl(0x02);

	unsigned char counter1 = '2';
	unsigned char counter2 = '4';
	unsigned char currentCounter1 = counter1;
	unsigned char currentCounter2 = counter2;

	dataCtrl('T');
	dataCtrl('I');
	dataCtrl('M');
	dataCtrl('E');
	dataCtrl(':');
	instCtrl(0xC0); // set cursor next line below TIME

	while (1)
	{
		if (RD4 == 0) // Check if keypad is high (pressed)
		{
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
				delay(61);
			}
			else
			{
				currentCounter2--;
			}

			clearDataResetCursor(); // move cursor and overwrite

			dataCtrl(currentCounter1);
			dataCtrl(currentCounter2);

			while (RD4 == 1)
				;
		}
	}
}
