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

// Postavke igre ------
#define GAME_SPEED 700
#define GAME_LENGTH 100
// --------------------

// Izbornik -----------
#define SIMON_ICON_X1 0
#define SIMON_ICON_Y1 0
#define SIMON_ICON_X2 159
#define SIMON_ICON_Y2 239
#define KRIUZIC_ICON_X1 160
#define KRIUZIC_ICON_Y1 0
#define KRIUZIC_ICON_X2 329
#define KRIUZIC_ICON_Y2 239
// --------------------

// krizic ------------
#define BOARD_X1 90		// left edge of the board
#define BOARD_X2 231	// right edge of the board
#define BOARD_Y1 43		// bottom edge of the board
#define BOARD_Y2 184	// top edge of the board

#define BORDER_X1 133	// left edge of left vertical line
#define BORDER_X2 182	// left edge of right vertical line
#define BORDER_Y1 86	// bottom edge of bottom horizontal line
#define BORDER_Y2 135	// bottom edge of top horizontal line

#define BORDER_WIDTH 6
#define SQUARE_WIDTH 43
// --------------------

// Gumb za povratak ---
#define BACK_X1 0
#define BACK_Y1 0
#define BACK_X2 55
#define BACK_Y2 30
#define BACK_TEXT_X 10
// --------------------

// Simon kvadrati -----
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
// --------------------

// Start gumb ---------
#define START_Y1 196
#define START_Y2 229
#define START_TEXT_Y 208
// --------------------

// Ostalo -------------
#define SCORE_Y 16
#define TOP_TEXT_Y 10
#define HI_X 258
#define HI_NUM_X 303
// --------------------

/* currentDisplay
 * 0 - MAIN MENU
 * 1 - SIMON
 * 2 - KRIZIC
 */

uint8_t currentDisplay = 0;

/* krizicEndFlag
 * 0 - igra jos traje
 * 1 - X je pobijedio
 * 2 - O je pobijedio
 * 3 - nerijeseno
 */

uint8_t tileSequence[GAME_LENGTH], highscore = 0, steps = 1, xWins = 0, oWins = 0, krizicEndFlag;
char moveHistory[3][3], turn, firstMove = 'O';
UTFT display;

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
	_delay_ms(10);
	return (uint16_t)x;
}

uint16_t getY() {
	SPI_Write(0b11010000);
	float y = SPI_Read() / 120.0 * 239;
	SPI_Write(0b00000000);
	_delay_ms(10);
	return (uint16_t)y;
}

// Crtanje Simon says komponenti
void simonDrawBoard() {
	display.clrScr();

	// Zelena plocica
	display.setColor(0, 191, 0);
	display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2); 
	
	// Crvena plocica	
	display.setColor(191, 0, 0);
	display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2); 

	// Žuta plocica
	display.setColor(191, 191, 0);
	display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2); 

	// Plava plocica
	display.setColor(0, 0, 191);
	display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2); 

	// Start gumb sirine igracih plocica
	display.setColor(255, 255, 255); 
	display.drawRect(GREEN_X1, START_Y1, RED_X2, START_Y2); 
	display.setFont(SmallFont);
	display.print("Kreni", CENTER, START_TEXT_Y);

	// Highscore prikaz
	display.print("Rekord:", HI_X, TOP_TEXT_Y);
	display.printNumI(highscore, HI_NUM_X, TOP_TEXT_Y);

	// Gumb za povratak
	display.print("Natrag", BACK_TEXT_X, TOP_TEXT_Y);
	
	// Prikaz broja koraka
	display.setFont(BigFont);
	display.printNumI(0, CENTER, SCORE_Y);
}

// Blinkanje plocica
void simonBlinkTile(uint8_t tileNumber) {
	_delay_ms(GAME_SPEED);
	switch (tileNumber) {
		case 1: // Zelena
		display.setColor(0, 255, 0);
		display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2);
		_delay_ms(GAME_SPEED);
		display.setColor(0, 191, 0);
		display.fillRect(GREEN_X1, GREEN_Y1, GREEN_X2, GREEN_Y2);
		break;
		
		case 2: // Crvena
		display.setColor(255, 0, 0);
		display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2);
		_delay_ms(GAME_SPEED);
		display.setColor(191, 0, 0);
		display.fillRect(RED_X1, RED_Y1, RED_X2, RED_Y2);
		break;
		
		case 3: // Zuta
		display.setColor(255, 255, 0);
		display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2);
		_delay_ms(GAME_SPEED);
		display.setColor(191, 191, 0);
		display.fillRect(YELLOW_X1, YELLOW_Y1, YELLOW_X2, YELLOW_Y2);
		break;
		
		case 4: // Plava
		display.setColor(0, 0, 255);
		display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2);
		_delay_ms(GAME_SPEED);
		display.setColor(0, 0, 191);
		display.fillRect(BLUE_X1, BLUE_Y1, BLUE_X2, BLUE_Y2);
		break;
	}
}

// Novi level
void simonNewLevel(uint8_t steps) {
	// Azuriranje broja koraka
	display.setColor(0, 0, 0);
	display.fillRect(GREEN_X1 - 5, 0, RED_X2 + 5, RED_Y1 - 1); 
	display.setColor(255, 255, 255); 
	display.setFont(BigFont);
	display.printNumI(steps, CENTER, SCORE_Y);

	tileSequence[steps - 1] = rand() % 4 + 1; // Novi nasumicni element
	
	// Blinkanje cijelog niza
	for (uint8_t i = 0; i < steps; i++) {
		simonBlinkTile(tileSequence[i]);
	}
}

// Provjera sto je pritisnuto
int simonCheckInput() {
	while (!isTouched());
	uint16_t x = getX();
	uint16_t y = getY();

	/* 
	 *-1 - Povratak
	 * 1 - Zelena plocica
	 * 2 - Crvena plocica
	 * 3 - Zuta plocica
	 * 4 - Plava plocica
	 * 5 - Start
	 * 0 - Nista od navedenog
	 */

	if ((x > BACK_X1) && (x < BACK_X2) && (y > BACK_Y1) && (y < BACK_Y2)) return -1;
	else if ((x > GREEN_X1) && (x < GREEN_X2) && (y > GREEN_Y1) && (y < GREEN_Y2)) return 1;
	else if ((x > RED_X1) && (x < RED_X2) && (y > RED_Y1) && (y < RED_Y2)) return 2;
	else if ((x > YELLOW_X1) && (x < YELLOW_X2) && (y > YELLOW_Y1) && (y < YELLOW_Y2)) return 3;
	else if ((x > BLUE_X1) && (x < BLUE_X2) && (y > BLUE_Y1) && (y < BLUE_Y2)) return 4;
	else if ((x > GREEN_X1) && (x < RED_X2) && (y > START_Y1) && (y < START_Y2)) return 5;
	else return 0;
}

// Provjera ispravnosti unesenog niza
int simonVerifySequence(uint8_t steps) {
	uint8_t input, index = 0;

	while (index < steps) {
		input = simonCheckInput();
		if ((input > 0) && (input < 5)) {
			if (input != tileSequence[index++]) return 0;
		} else if(input < 0){
            return 0;
        }
		_delay_ms(100); // Debounce cekanjem
	}
	return 1;
}

// Inicijalizacija igre Simon says
void simonStart() {
	steps = 1;
	// Brisanje Start gumba
	display.setColor(0, 0, 0);
	display.fillRect(GREEN_X1, START_Y1, RED_X2, START_Y2); 
	_delay_ms(500);
	simonNewLevel(1); // Dodavanje prvog elementa u niz
}

// Kraj igre Simon says
void simonGameOver(uint8_t win) {
	// Azuriranje highscora
	display.setColor(0, 0, 0);
	if (steps - 1 > highscore) {
		highscore = steps - 1;
		display.fillRect(HI_NUM_X - 1, TOP_TEXT_Y - 2, 319, TOP_TEXT_Y + 15);
		display.printNumI(highscore, HI_NUM_X, TOP_TEXT_Y);
	}

	// Ispis poruke o kraju igre
	display.fillRect(GREEN_X1 - 5, 0, RED_X2 + 5, RED_Y1 - 1);
	display.setFont(SmallFont);
	if (win) {
		display.print("Pobjeda!", CENTER, SCORE_Y);
	} else {
		display.print("Pogresno! Pokusaj ponovo.", CENTER, SCORE_Y);
	}

	// Prikaz start gumba
	display.setColor(255, 255, 255); 
	display.drawRect(GREEN_X1, START_Y1, RED_X2, START_Y2);
	display.setFont(SmallFont);
	display.print("Kreni", CENTER, START_TEXT_Y);
}

// Izvodenje igre
void simonGame() {
	uint8_t win = 0;
	simonDrawBoard();
	while (simonCheckInput() != 5);
	simonStart();
	while (simonVerifySequence(steps)) {
		++steps;
		if (steps > GAME_LENGTH) {
			win = 1;
			break;
		} else {
			simonNewLevel(steps);
		}
	}
	simonGameOver(win);
}

// new game button
void krizicNovaIgra() {
    display.setColor(255, 255, 255); 
    display.drawRect(BOARD_X1, START_Y1, BOARD_X2, START_Y2);
    display.setFont(SmallFont);
    display.print("Nova igra", CENTER, START_TEXT_Y);
}

void backButton() {
    display.setColor(255, 255, 255); 
    display.drawRect(BACK_X1, BACK_Y1, BACK_X2, BACK_Y2);
    display.setFont(SmallFont);
    display.print("<", BACK_TEXT_X, TOP_TEXT_Y);
}

// draws board and initialises global variables
void krizicInit() {

	// draw board
	display.clrScr();
    display.setFont(BigFont);

	display.setColor(255, 255, 255);
	display.fillRect(BORDER_X1, BOARD_Y1, BORDER_X1 + BORDER_WIDTH, BOARD_Y2);
	display.fillRect(BORDER_X2, BOARD_Y1, BORDER_X2 + BORDER_WIDTH, BOARD_Y2);
	display.fillRect(BOARD_X1, BORDER_Y1, BOARD_X2, BORDER_Y1 + BORDER_WIDTH);
	display.fillRect(BOARD_X1, BORDER_Y2, BOARD_X2, BORDER_Y2 + BORDER_WIDTH);

	display.print("X:", HI_X + 20, TOP_TEXT_Y);
	display.printNumI(xWins, HI_X + 40, TOP_TEXT_Y);
	display.print("O:", HI_X + 20, TOP_TEXT_Y + 20);
	display.printNumI(oWins, HI_X + 40, TOP_TEXT_Y + 20);

	// init variables
	uint8_t i, j;
	firstMove = firstMove == 'X' ? 'O' : 'X';  // alternating first move
    turn = firstMove;
	krizicEndFlag = 0;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j) {
			moveHistory[i][j] = '0';
		}
	}
}

// checks which square is pressed
uint8_t krizicCheckInput() {
	while (!isTouched());
	uint16_t x = getX();
	uint16_t y = getY();

	/*
	 *-1 - Povratak
	 * 1 - Gore lijevo
	 * 2 - Gore sredina
	 * 3 - Gore desno
	 * 4 - Lijevo
	 * 5 - Sredina
	 * 6 - Desno
	 * 7 - Dolje lijevo
	 * 8 - Dolje sredina
	 * 9 - Dolje desno
     *10 - Nova igra
	 * 0 - Ništa od navedenog
	 */

	if ((x > BACK_X1) && (x < BACK_X2) && (y > BACK_Y1) && (y < BACK_Y2)) return -1;
    else if ((x > BOARD_X1) && (x < BOARD_X2) && (y > START_Y1) && (y < START_Y2)) return 10;

	else if ((x > BOARD_X1)                 && (x < BORDER_X1)  && (y > BORDER_Y2 + BORDER_WIDTH) && (y < BOARD_Y2)) return 1;
	else if ((x > BORDER_X1 + BORDER_WIDTH) && (x < BORDER_X2)  && (y > BORDER_Y2 + BORDER_WIDTH) && (y < BOARD_Y2)) return 2;
	else if ((x > BORDER_X2 + BORDER_WIDTH) && (x < BOARD_X2)   && (y > BORDER_Y2 + BORDER_WIDTH) && (y < BOARD_Y2)) return 3;

	else if ((x > BOARD_X1)                 && (x < BORDER_X1)  && (y > BORDER_Y1 + BORDER_WIDTH) && (y < BORDER_Y2)) return 4;
	else if ((x > BORDER_X1 + BORDER_WIDTH) && (x < BORDER_X2)  && (y > BORDER_Y1 + BORDER_WIDTH) && (y < BORDER_Y2)) return 5;
	else if ((x > BORDER_X2 + BORDER_WIDTH) && (x < BOARD_X2)   && (y > BORDER_Y1 + BORDER_WIDTH) && (y < BORDER_Y2)) return 6;

	else if ((x > BOARD_X1)                 && (x < BORDER_X1)  && (y > BOARD_Y1)                 && (y < BORDER_Y1)) return 7;
	else if ((x > BORDER_X1 + BORDER_WIDTH) && (x < BORDER_X2)  && (y > BOARD_Y1)                 && (y < BORDER_Y1)) return 8;
	else if ((x > BORDER_X2 + BORDER_WIDTH) && (x < BOARD_X2)   && (y > BOARD_Y1)                 && (y < BORDER_Y1)) return 9;

	else return 0;
}

// verify that the pressed square was available
// saves move (turn) to moveHistory
// returns 1 if the chosen square is empty (value '0') or 0 if it is occupied (values 'X' and 'O')
uint8_t krizicVerifyInput(uint8_t input) {
	uint8_t i, j, squareNum = 0;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j) {
			squareNum++;

			if (squareNum == input && moveHistory[i][j] == '0') {
				moveHistory[i][j] = turn;
				return 1;

			} else if (squareNum == input && moveHistory[i][j] != '0') return 0;
		}
	}

	return 0;
}

// prints moves
void krizicDrawInput(uint8_t input) {
    display.setColor(255, 255, 255);
    display.setFont(BigFont);

    uint8_t x, y;

    if (input > 0 && input < 4) {           // top squares
        y = BORDER_Y2 + BORDER_WIDTH;
    } else if (input > 3 && input < 7) {    // middle (horizontal)
        y = BORDER_Y1 + BORDER_WIDTH;
    } else if (input > 6 && input < 10) {   // bottom
        y = BOARD_Y1;
    }

    if ((input + 2) % 3 == 0) {             // left squares
        x = BOARD_X1;
    } else if ((input + 1) % 3 == 0) {      // middle (vertical)
        x = BORDER_X1 + BORDER_WIDTH;
    } else if (input % 3 == 0) {            // right
        x = BORDER_X2 + BORDER_WIDTH;
    }

	char trn[2] = {turn, '\0'};
	display.print(trn, x + 14, y + 14);

    _delay_ms(500);
}

// checks if game is over
void krizicCheckEndGame() {
	uint8_t i, j;
	
	if (moveHistory[0][0] == moveHistory[0][1] && moveHistory[0][0] == moveHistory[0][2] && moveHistory[0][0] != '0' ||	// rows
		moveHistory[1][0] == moveHistory[1][1] && moveHistory[1][0] == moveHistory[1][2] && moveHistory[1][0] != '0' ||
		moveHistory[2][0] == moveHistory[2][1] && moveHistory[2][0] == moveHistory[2][2] && moveHistory[2][0] != '0' ||

		moveHistory[0][0] == moveHistory[1][0] && moveHistory[0][0] == moveHistory[2][0] && moveHistory[0][0] != '0' ||	// columns
		moveHistory[0][1] == moveHistory[1][1] && moveHistory[0][1] == moveHistory[2][1] && moveHistory[0][1] != '0' ||
		moveHistory[0][2] == moveHistory[1][2] && moveHistory[0][2] == moveHistory[2][2] && moveHistory[0][2] != '0' ||

		moveHistory[0][0] == moveHistory[1][1] && moveHistory[0][0] == moveHistory[2][2] && moveHistory[0][0] != '0' ||	// diagonals
		moveHistory[0][2] == moveHistory[1][1] && moveHistory[0][2] == moveHistory[2][0] && moveHistory[0][2] != '0'){
		
		// only the last player to make a move can win
		krizicEndFlag = turn == 'X' ? 1 : 2;
	}

	// checks if it's a tie (all squares will be used, but there won't be a winner)
	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j) {
			if (moveHistory[i][j] == '0') return;
		}
	}
	krizicEndFlag = 3;
}

// handles game over
void krizicGameOver() {
    display.setColor(0, 0, 0);
    display.fillRect(BOARD_X1, BORDER_Y1, BOARD_X2, BORDER_Y2);
    display.setFont(SmallFont);

	display.setColor(255, 255, 255);
	switch (krizicEndFlag) {
		case 1:		//	X won
			xWins++;
            display.print("X je pobjednik!", CENTER, TOP_TEXT_Y);
			break;
		case 2:		// O won
			oWins++;
            display.print("O je pobjednik!", CENTER, TOP_TEXT_Y);
			break;
		case 3:		// tie
            display.print("Nerijeseno!", CENTER, TOP_TEXT_Y);
			break;
	}

    krizicNovaIgra();
}

// main game function
void krizicGame() {
    uint8_t input;
    krizicNovaIgra();

    while (krizicCheckInput() == 10) {
    	krizicInit();

    	while (!krizicEndFlag) {
    		input = krizicCheckInput();

    		if (input < 0) {
    			return;
    		} else if (input > 0) {
    			if (krizicVerifyInput(input)) {
    				krizicDrawInput(input);
    				krizicCheckEndGame();
                    turn = turn == 'X' ? 'O' : 'X';
    			}
    		}
    	}

    	krizicGameOver();
    }
}

void openGame() {
	uint16_t x = getX();
	uint16_t y = getY();

	if ((x > SIMON_ICON_X1) && (x < SIMON_ICON_X2) && (y > SIMON_ICON_Y1) && (y < SIMON_ICON_Y2)) {
		currentDisplay = 1;
	} else if ((x > KRIUZIC_ICON_X1) && (x < KRIUZIC_ICON_X2) && (y > KRIUZIC_ICON_Y1) && (y < KRIUZIC_ICON_Y2)) {
		currentDisplay = 2;
	}
}

int main(void) {
	// PINB2 ulazni - spojeno na pin T_IRQ touch controllera (normally high, low when touching)
	DDRB &= 0b11110111;
	
	SPI_Init();
	SS_Enable;
	
	display.InitLCD(LANDSCAPE);
	display.setFont(BigFont);
	display.clrScr();
	
	while (1) {
		if (currentDisplay == 0) {
			while (!isTouched());
			openGame();
		} else if (currentDisplay == 1) {
			simonGame();
		} else if (currentDisplay == 2) {
			krizicGame();
		}
	}
}