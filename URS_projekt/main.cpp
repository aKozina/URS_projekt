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

#define GREEN_X1 87
#define GREEN_Y1 40
#define GREEN_X2 157
#define GREEN_Y2 110

#define RED_X1 163
#define RED_Y1 40
#define RED_X2 233
#define RED_Y2 110

#define YELLOW_X1 87
#define YELLOW_Y1 116
#define YELLOW_X2 157
#define YELLOW_Y2 186

#define BLUE_X1 163
#define BLUE_Y1 116
#define BLUE_X2 233
#define BLUE_Y2 186

#define START_Y1 196
#define START_Y2 229

#define START_TEXT_Y 208

#define SCORE_Y 16

#define BACK_X1 0
#define BACK_Y1 0
#define BACK_X2 55
#define BACK_Y2 30

#define TOP_TEXT_Y 10

#define HI_X 258
#define HI_NUM_X 303

#define GAME_SPEED 700
#define GAME_LENGTH 255

uint8_t tileSequence[GAME_LENGTH], highscore = 0, steps = 1;

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

void drawBoard() {
	display.clrScr();

	display.setColor(0, 128, 0);
	display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2); // Zelena pločica

	display.setColor(128, 0, 0);
	display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2); // Crvena pločica

	display.setColor(128, 128, 0);
	display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2); // Žuta pločica

	display.setColor(0, 0, 128);
	display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2); // Plava pločica

	display.setColor(255, 255, 255); 
	display.drawRect(GREEN_X1, START_Y1, RED_X2, START_Y2); // Start gumb širine igraćih pločica
	display.setFont(SmallFont);
	display.print("Kreni", CENTER, START_TEXT_Y);

	display.print("Rekord:", HI_X, TOP_TEXT_Y);
	display.printNumI(highscore, HI_NUM_X, TOP_TEXT_Y);
	
	display.setFont(BigFont);
	display.printNumI(0, CENTER, SCORE_Y);
}

void newSequence(uint8_t steps) {
	display.setColor(0, 0, 0);
	display.fillRect(GREEN_X1 - 5, 0, RED_X2 + 5, RED_Y1 - 1); // Brisanje prethodnog broja koraka

	display.setColor(255, 255, 255); 
	display.setFont(BigFont);
	display.printNumI(steps, CENTER, SCORE_Y); // Novi broj koraka

	for (uint8_t i = 0; i < steps; i++) {
		uint8_t tileNumber = rand() % 4 + 1;
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

int checkInput() {
	while (!isTouched());
	uint16_t x = getX();
	uint16_t y = getY();

	/* 
	 *-1 - Gumb za povratak
	 * 1 - Zelena pločica
	 * 2 - Crvena pločica
	 * 3 - Žuta pločica
	 * 4 - Plava pločica
	 * 5 - Start gumb
	 * 0 - Ništa od navedenog
	 */

	if ((x > BACK_X1) && (x < BACK_X2) && (y > BACK_Y1) && (y < BACK_Y2)) return -1;
	else if ((x > GREEN_X1) && (x < GREEN_X2) && (y > GREEN_Y1) && (y < GREEN_Y2)) return 1;
	else if ((x > RED_X1) && (x < RED_X2) && (y > RED_Y1) && (y < RED_Y2)) return 2;
	else if ((x > YELLOW_X1) && (x < YELLOW_X2) && (y > YELLOW_Y1) && (y < YELLOW_Y2)) return 3;
	else if ((x > BLUE_X1) && (x < BLUE_X2) && (y > BLUE_Y1) && (y < BLUE_Y2)) return 4;
	else if ((x > GREEN_X1) && (x < RED_X2) && (y > START_Y1) && (y < START_Y2)) return 5;
	else return 0;
	
}

int verifySequence(uint8_t steps) {
	uint8_t input, index = 0;
	while (index < steps) {
		input = checkInput();
		if ((input > 0) && (input < 5)) {
			if (input != tileSequence[index++]) return 0;
		}
	}
	return 1;
}

void startGame() {
	steps = 1;
	display.setColor(0, 0, 0);
	display.fillRect(GREEN_X1, START_Y1, RED_X2, START_Y2); // Brisanje Start gumba
	_delay_ms(500);
	newSequence(1);
}

void gameOver() {
	display.setColor(0, 0, 0);
	if (steps - 1 > highscore) {
		highscore = steps - 1;
		display.fillRect(HI_NUM_X - 1, TOP_TEXT_Y - 2, 319, TOP_TEXT_Y + 15);
		display.printNumI(highscore, HI_NUM_X, TOP_TEXT_Y);
	}
	display.fillRect(GREEN_X1 - 5, 0, RED_X2 + 5, RED_Y1 - 1);
	display.setFont(SmallFont);
	display.print("Pogresno! Pokusaj ponovo.", CENTER, SCORE_Y);

	display.setColor(255, 255, 255); 
	display.drawRect(GREEN_X1, START_Y1, RED_X2, START_Y2); // Start gumb
	display.setFont(SmallFont);
	display.print("Kreni", CENTER, START_TEXT_Y);
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
	
	drawBoard();

	while (1) {
		while (checkInput() != 5);
			startGame();
			while(verifySequence(steps)) {
				newSequence(++steps);
			}
			gameOver();
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
