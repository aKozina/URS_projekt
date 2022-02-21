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

#define GREEN_X1 97
#define GREEN_Y1 57
#define GREEN_X2 157
#define GREEN_Y2 117

#define RED_X1 163
#define RED_Y1 57
#define RED_X2 223
#define RED_Y2 117

#define YELLOW_X1 97
#define YELLOW_Y1 123
#define YELLOW_X2 157
#define YELLOW_Y2 183

#define BLUE_X1 163
#define BLUE_Y1 123
#define BLUE_X2 223
#define BLUE_Y2 183

#define GAME_SPEED 700

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



void drawBoard() { // Crtanje četiri kvadrata
	display.clrScr();

	display.setColor(0, 128, 0);
	display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2); // Zeleni

	display.setColor(128, 0, 0);
	display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2); // Crveni

	display.setColor(128, 128, 0);
	display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2); // Žuti

	display.setColor(0, 0, 128);
	display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2); // Plavi
}

void newSequence(uint8_t steps) {
	for (uint8_t i = 0; i < steps; i++) {
		uint8_t tileNumber = rand() % 4;
		tileSequence[i] = tileNumber;
		blinkTile(tileNumber);
		_delay_ms(GAME_SPEED);
	}
}

void blinkTile(uint8_t tileNumber) {
	switch tileNumber {
		case 0:
			display.setColor(0, 255, 0);
			display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2);
			_delay_ms(GAME_SPEED);
			display.setColor(0, 128, 0);
			display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2);
			break;
		
		case 1:
			display.setColor(255, 0, 0);
			display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2);
			_delay_ms(GAME_SPEED);
			display.setColor(128, 0, 0);
			display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2);
			break;
		
		case 2:
			display.setColor(255, 255, 0);
			display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2);
			_delay_ms(GAME_SPEED);
			display.setColor(128, 128, 0);
			display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2);	
			break;
		
		case 3:
			display.setColor(0, 0, 255);
			display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2);
			_delay_ms(GAME_SPEED);
			display.setColor(0, 0, 128);
			display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2);
			break;
	}
}

	while (1) {
		uint8_t tileSequence[255];
		uint8_t steps = 1;
		uint16_t k = 0;
		
		while (1) {
			blinkTile(k++);
			_delay_ms(GAME_SPEED);
			if (k == 3) k = 0;
		}

		/*
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
		*/
	}
}
