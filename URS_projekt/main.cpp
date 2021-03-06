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
#define SIMON_TEXT_X 40
#define SIMON_TEXT_Y 110
#define KRIUZIC_ICON_X1 160
#define KRIUZIC_ICON_Y1 0
#define KRIUZIC_ICON_X2 319
#define KRIUZIC_ICON_Y2 239
#define KRIUZIC_TEXT_X1 190
#define KRIUZIC_TEXT_Y1 100
#define KRIUZIC_TEXT_X2 200
#define KRIUZIC_TEXT_Y2 120
// --------------------

// Krizic ploca -------
#define BOARD_X1 90         // lijevi rub ploce
#define BOARD_X2 231        // desni rub ploce
#define BOARD_Y1 43         // donji rub ploce
#define BOARD_Y2 184        // gornji rub ploce

#define BORDER_X1 133       // lijevi rub lijeve vertikalne linije
#define BORDER_X2 182       // lijevi rub desne vertikalne linije
#define BORDER_Y1 86        // donji rub donje horizontalne linije
#define BORDER_Y2 135       // donji rub gornje horizontalne linije

#define BORDER_WIDTH 6      // sirinahorizontalnih i vertikalnih linija
#define SQUARE_WIDTH 43     // sirina kvadrata
// --------------------

// Gumb za povratak ---
#define BACK_X1 0
#define BACK_Y1 0
#define BACK_X2 40
#define BACK_Y2 40
#define BACK_TEXT_X 15
#define BACK_TEXT_Y 15
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
#define HI_X 250
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

void backButton() {
    display.setColor(255, 255, 255);
    display.drawRect(BACK_X1, BACK_Y1, BACK_X2, BACK_Y2);
    display.setFont(SmallFont);
    display.print("<", BACK_TEXT_X, BACK_TEXT_Y);
}

void drawMenu() {
    display.clrScr();
    display.setColor(255, 255, 255);
    display.drawRect(SIMON_ICON_X1, SIMON_ICON_Y1, SIMON_ICON_X2, SIMON_ICON_Y2);
    display.setColor(255, 255, 255);
    display.drawRect(KRIUZIC_ICON_X1, KRIUZIC_ICON_Y1, KRIUZIC_ICON_X2, KRIUZIC_ICON_Y2);
    display.setFont(BigFont);
    display.print("Simon", SIMON_TEXT_X, SIMON_TEXT_Y);
    display.print("Krizic-", KRIUZIC_TEXT_X1, KRIUZIC_TEXT_Y1);
    display.print("Kruzic", KRIUZIC_TEXT_X2, KRIUZIC_TEXT_Y2);
}

void resetValues() {
    uint8_t i, j;
    xWins = 0, oWins = 0;
    turn = 'X';
    krizicEndFlag = 0;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            moveHistory[i][j] = '0';
        }
    }
    for (i = 0; i < GAME_LENGTH; i++) {
        tileSequence[i] = -1;
    }
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

    // ??uta plocica
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
    backButton();

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
uint8_t simonCheckInput() {
    while (!isTouched());
    uint16_t x = getX();
    uint16_t y = getY();

    /*
     * 100 - Povratak
     * 1 - Zelena plocica
     * 2 - Crvena plocica
     * 3 - Zuta plocica
     * 4 - Plava plocica
     * 5 - Start
     * 0 - Nista od navedenog
     */

    if ((x > BACK_X1) && (x < BACK_X2) && (y > BACK_Y1) && (y < BACK_Y2)) return 100;
    else if ((x > GREEN_X1) && (x < GREEN_X2) && (y > GREEN_Y1) && (y < GREEN_Y2)) return 1;
    else if ((x > RED_X1) && (x < RED_X2) && (y > RED_Y1) && (y < RED_Y2)) return 2;
    else if ((x > YELLOW_X1) && (x < YELLOW_X2) && (y > YELLOW_Y1) && (y < YELLOW_Y2)) return 3;
    else if ((x > BLUE_X1) && (x < BLUE_X2) && (y > BLUE_Y1) && (y < BLUE_Y2)) return 4;
    else if ((x > GREEN_X1) && (x < RED_X2) && (y > START_Y1) && (y < START_Y2)) return 5;
    else return 0;
}

// Provjera ispravnosti unesenog niza
uint8_t simonVerifySequence(uint8_t steps) {
    uint8_t input, index = 0;

    while (index < steps) {
        input = simonCheckInput();
        if ((input > 0) && (input < 5)) {
            if (input != tileSequence[index++]) return 0;
        } else if(input == 0){
            return 0;
        } else if (input == 100) {
            currentDisplay = 0;
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
    uint8_t win = 0, input;
    simonDrawBoard();
    while (1) {
        input = simonCheckInput();
        if (input == 100) {
            currentDisplay = 0;
            return;
        } else if (input == 5) {
            break;
        }
    }
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

// Gumb za pocetak igre
void krizicNovaIgra() {
    display.setColor(255, 255, 255);
    display.drawRect(BOARD_X1, START_Y1, BOARD_X2, START_Y2);
    display.setFont(SmallFont);
    display.print("Nova igra", CENTER, START_TEXT_Y);
}

// Crtanje ploce i iinicijalizacija varijabli
void krizicInit() {

    // Crtanje ploce
    display.clrScr();
    display.setFont(BigFont);

    backButton();

    display.setColor(255, 255, 255);
    display.fillRect(BORDER_X1, BOARD_Y1, BORDER_X1 + BORDER_WIDTH, BOARD_Y2);
    display.fillRect(BORDER_X2, BOARD_Y1, BORDER_X2 + BORDER_WIDTH, BOARD_Y2);
    display.fillRect(BOARD_X1, BORDER_Y1, BOARD_X2, BORDER_Y1 + BORDER_WIDTH);
    display.fillRect(BOARD_X1, BORDER_Y2, BOARD_X2, BORDER_Y2 + BORDER_WIDTH);

    display.print("X:", HI_X + 20, TOP_TEXT_Y);
    display.printNumI(xWins, HI_X + 40, TOP_TEXT_Y);
    display.print("O:", HI_X + 20, TOP_TEXT_Y + 20);
    display.printNumI(oWins, HI_X + 40, TOP_TEXT_Y + 20);

    // Inicijalizacija varijabli
    uint8_t i, j;
    firstMove = firstMove == 'X' ? 'O' : 'X';  // Izmjenicni prvi potez
    turn = firstMove;
    krizicEndFlag = 0;

    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            moveHistory[i][j] = '0';
        }
    }
}

// Provjera koji kvadrat/gumb je pritisnut
uint8_t krizicCheckInput() {
    while (!isTouched());
    uint16_t x = getX();
    uint16_t y = getY();

    /*
     * 100 - Povratak
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
     * 0 - Ni??ta od navedenog
     */

    if ((x > BACK_X1) && (x < BACK_X2) && (y > BACK_Y1) && (y < BACK_Y2)) return 100;
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

// Provjerava je li odabrani kvadrat dostupan
// Sprema potez u moveHistory
// Vraca 1 ako je odabrani kvadrat prazan/dostupan (vrijednost '0') ili 0 ako je zauzet (vrijednosti 'X' i 'O')
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

// Ispisuje poteze
void krizicDrawInput(uint8_t input) {
    display.setColor(255, 255, 255);
    display.setFont(BigFont);

    uint8_t x = 0, y = 0;

    if (input > 0 && input < 4) {           // Gornji red kvadrata
        y = BORDER_Y2 + BORDER_WIDTH;
    } else if (input > 3 && input < 7) {    // Srednji red
        y = BORDER_Y1 + BORDER_WIDTH;
    } else if (input > 6 && input < 10) {   // Donji red
        y = BOARD_Y1;
    }

    if ((input + 2) % 3 == 0) {             // Lijevi stupac
        x = BOARD_X1;
    } else if ((input + 1) % 3 == 0) {      // Srednji stupac
        x = BORDER_X1 + BORDER_WIDTH;
    } else if (input % 3 == 0) {            // Desni stupac
        x = BORDER_X2 + BORDER_WIDTH;
    }

    char trn[2] = {turn, '\0'};
    display.print(trn, x + 14, y + 14);

    _delay_ms(500);     // Debounce cekanjem
}

// Provjerava je li igra gotova
void krizicCheckEndGame() {
    uint8_t i, j;

    if ((moveHistory[0][0] == moveHistory[0][1]) && (moveHistory[0][0] == moveHistory[0][2]) && (moveHistory[0][0] != '0') ||	// Retci
        (moveHistory[1][0] == moveHistory[1][1]) && (moveHistory[1][0] == moveHistory[1][2]) && (moveHistory[1][0] != '0') ||
        (moveHistory[2][0] == moveHistory[2][1]) && (moveHistory[2][0] == moveHistory[2][2]) && (moveHistory[2][0] != '0') ||

        (moveHistory[0][0] == moveHistory[1][0]) && (moveHistory[0][0] == moveHistory[2][0]) && (moveHistory[0][0] != '0') ||	// Stupci
        (moveHistory[0][1] == moveHistory[1][1]) && (moveHistory[0][1] == moveHistory[2][1]) && (moveHistory[0][1] != '0') ||
        (moveHistory[0][2] == moveHistory[1][2]) && (moveHistory[0][2] == moveHistory[2][2]) && (moveHistory[0][2] != '0') ||

        (moveHistory[0][0] == moveHistory[1][1]) && (moveHistory[0][0] == moveHistory[2][2]) && (moveHistory[0][0] != '0') ||	// Dijagonale
        (moveHistory[0][2] == moveHistory[1][1]) && (moveHistory[0][2] == moveHistory[2][0]) && (moveHistory[0][2] != '0')){

        // Samo igrac koji je zadnji odigrao potez moze pobijediti
        krizicEndFlag = turn == 'X' ? 1 : 2;
        return;
    }

    // Provjerava je li nerijeseno (svi kvadrati su iskoristeni, a nema pobjednika)
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            if (moveHistory[i][j] == '0') return;
        }
    }
    krizicEndFlag = 3;
}

// Ispisuje poruku kad igra zavrsi i broji pobjede
void krizicGameOver() {
    display.setFont(SmallFont);
    display.setColor(255, 255, 255);
    switch (krizicEndFlag) {
        case 1:		//	X je pobjednik
            xWins++;
            display.print("X je pobjednik!", CENTER, TOP_TEXT_Y);
            break;
        case 2:		// O je pobjednik
            oWins++;
            display.print("O je pobjednik!", CENTER, TOP_TEXT_Y);
            break;
        case 3:		// Nerijeseno
            display.print("Nerijeseno!", CENTER, TOP_TEXT_Y);
            break;
    }

    krizicNovaIgra();
}

// Glavna funkcija za krizic kruzic
void krizicGame() {
    uint8_t input, waiting = 1;
    krizicNovaIgra();

    while (1) {
        input = krizicCheckInput();
        if (input == 100) {
            currentDisplay = 0;
            return;
        } else if (input == 10 && waiting) {
            krizicInit();
            waiting = 0;
        } else if (input > 0 && !waiting) {
            if (krizicVerifyInput(input)) {
                krizicDrawInput(input);
                krizicCheckEndGame();
                turn = turn == 'X' ? 'O' : 'X';
                if (krizicEndFlag) {
                    break;
                }
            }
        }
        _delay_ms(50);
    }

    krizicGameOver();
}

void openGame() {
    uint16_t x = getX();
    uint16_t y = getY();

    if ((x > SIMON_ICON_X1) && (x < SIMON_ICON_X2) && (y > SIMON_ICON_Y1) && (y < SIMON_ICON_Y2)) {
        currentDisplay = 1;
    } else if ((x > KRIUZIC_ICON_X1) && (x < KRIUZIC_ICON_X2) && (y > KRIUZIC_ICON_Y1) && (y < KRIUZIC_ICON_Y2)) {
        currentDisplay = 2;
    }
    display.clrScr();
    _delay_ms(100);
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
            resetValues();
            drawMenu();
            _delay_ms(100);
            while (!isTouched());
            openGame();
        } else if (currentDisplay == 1) {
            simonGame();
        } else if (currentDisplay == 2) {
            krizicGame();
        }
    }
}