#define F_CPU 7372800UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "AVR_UTFT/UTFT.h"
#include "AVR_UTFT/color.h"
#include "AVR_UTFT/had.c"
#include "AVR_UTFT/DefaultFonts.h"
#include "SPI_Master_H_file.h"

bool isTouched() {
	// PINB2 low = touched
	if (~PINB & 0b00001000) {
		return true;
	} else {
		return false;
	}
}

uint16_t getX() {
	SPI_Write(0b10010000);
	float x = SPI_Read() / 120.0 * 319;
	SPI_Write(0b00000000);
	delay(10);
	return (uint16_t)x;
}

uint16_t getY() {
	SPI_Write(0b11010000);
	float y = SPI_Read() / 120.0 * 239;
	SPI_Write(0b00000000);
	delay(10);
	return (uint16_t)y;
}

int main(void) {
	
	// PINB2 ulazni - spojeno na pin T_IRQ touch controllera (normally high, low when touching)
	DDRB &= 0b11110111;
	
	SPI_Init();
	SS_Enable;
	
	UTFT display;
	display.InitLCD(LANDSCAPE);
	display.setFont(BigFont);
	display.clrScr();
	
	display.setColor(255, 250, 0);
	
	uint16_t pos = 0;
	
	while (1) {
		if (isTouched()) {
			display.setColor(255, 0, 0);
			display.fillCircle(getX(), getY(), 10);
			delay(100);
		}
		
		display.setColor(255, 250, 0);
		display.fillRect(pos, 0, pos+19, 19);
		if (pos > 300) {
			pos = 0;
			display.setColor(0, 0, 0);
			display.fillRect(0, 0, 319, 19);
		} else {
			pos+=20;
		}
		
		delay(100);
	}
}
