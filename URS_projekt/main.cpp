#define F_CPU 7372800UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AVR_UTFT/UTFT.h"
#include "AVR_UTFT/color.h"
#include "AVR_UTFT/had.c"
#include "AVR_UTFT/DefaultFonts.h"

int main(void) {
	
	UTFT display;
	display.InitLCD(LANDSCAPE);
	display.setFont(BigFont);
	display.clrScr();
	
	display.setColor(255, 250, 0);
	
	uint8_t pos = 0;
	
	while (1) {
		display.fillRect(pos, 0, pos+20, 20);
		if (pos > 200) {
			pos = 0;
			display.clrScr();
		} else {
			pos+=20;
		}
		delay(100);
	}
}
