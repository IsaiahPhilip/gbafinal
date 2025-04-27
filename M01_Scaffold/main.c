#include "gba.h"
#include "mode0.h"
#include "sprites.h"
#include "game.h"
#include "print.h"
#include "intro.h"
#include "instructions.h"
#include "pausetilemap.h"
#include "pause.h"
#include "lose.h"
#include "win.h"
#include "digitalSound.h"
#include "intromusic.h"
#include "winscreen2.h"

enum
{
    START,
    INSTRUCTIONS,
    GAME,
    PAUSE,
    WIN, 
    LOSE
} state;

// random seed
int rSeed;

// random prototype
void srand();
void initialize();

// state prototypes
void goToStart();
void start();
void goToInstructions();
void instructions();
void goToLose();
void lose();
void goToGame();
void game();
void goToWin();
void win();
void goToPause();
void pause();

int main() {
    initialize();

    while(1) {
        oldButtons = buttons;
        buttons = REG_BUTTONS;
        switch(state) {
            case START:
                start();
                break;
            case INSTRUCTIONS:
                instructions();
                break;
            case GAME:
                game();
                break;
            case PAUSE:
                pause();
                break;
            case LOSE:
                lose();
                break;
            case WIN:
                win();
                break;
        }
    }
}

void initialize() {

    buttons = REG_BUTTONS;
    oldButtons = 0;

    initSound();
    mgba_open();
    goToStart();
    setupSounds();
}

void goToStart() {
    if(!(REG_DISPCTL & DISP_BACKBUFFER)) {
        flipPages();
    }

    REG_DISPCTL = MODE(4) | BG_ENABLE(2) | DISP_BACKBUFFER;

    drawFullscreenImage4(introBitmap);
    
    waitForVBlank();
    flipPages();
    DMANow(3, introPal, BG_PALETTE, 256);

    if(state!=INSTRUCTIONS) {
        stopSounds();
        playSoundA(intromusic_data, intromusic_length, 1);
    }

    state = START;

    // begin the seed randomization
    rSeed = 0;
}

// runs every frame of the start state
void start() {
    rSeed++;
    
    // locking frame rate to 60fps
    waitForVBlank();

    // // start to go to game
    // if (BUTTON_PRESSED(BUTTON_START)) {
    //     srand(rSeed); 
    //     goToGame();
    //     initGame();
    // }

    // select to toggle scoreboard
    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        goToInstructions();
    }
    if (BUTTON_PRESSED(BUTTON_START)) {
        goToGame();
    }
}

void goToInstructions() {
    drawFullscreenImage4(instructionsBitmap);
    
    waitForVBlank();
    flipPages();
    DMANow(3, instructionsPal, BG_PALETTE, 256);

    state = INSTRUCTIONS;
}

void instructions() {
    waitForVBlank();

    if (BUTTON_PRESSED(BUTTON_SELECT)) {
        goToStart();
    }
}

void game() {
    // if(currentLevel > 1) {
    //     goToWin();
    // }
    updateLevel();
    drawLevel();


    if(BUTTON_PRESSED(BUTTON_START)) {
        goToPause();
    }
}

void goToGame() {
    stopSounds();

    if(state == PAUSE) {
        initMap();
    } else {
        REG_DISPCTL = MODE(0) | BG_ENABLE(2) | BG_ENABLE(3) | SPRITE_ENABLE;
        REG_BG2CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(27) | BG_SIZE_WIDE;
        REG_BG3CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(29) | BG_SIZE_WIDE;
        

        initGame();
        initMap();
        initLevel();
    }

    state = GAME;
}

void pause() {
    if(BUTTON_PRESSED(BUTTON_START)) {
        goToGame();
    }
    if(BUTTON_PRESSED(BUTTON_SELECT)) {
        goToStart();
    }
}

void goToPause() {
    hideSprites();

    REG_BG2HOFF = 0;
    REG_BG2VOFF = 0;

    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128*4);
    DMANow(3, pauseMap, &SCREENBLOCK[27], pauseLen / 2);

    state = PAUSE;
}

void win() {
    if(BUTTON_PRESSED(BUTTON_START)) {
        goToStart();
    }
}

void goToWin() {
    hideSprites();

    REG_BG2HOFF = 0;
    REG_BG2VOFF = 0;

    if(!(REG_DISPCTL & DISP_BACKBUFFER)) {
        flipPages();
    }

    REG_DISPCTL = MODE(4) | BG_ENABLE(2) | DISP_BACKBUFFER;

    drawFullscreenImage4(winscreen2Bitmap);
    
    waitForVBlank();
    flipPages();
    DMANow(3, winscreen2Pal, BG_PALETTE, 256);

    // if(state!=INSTRUCTIONS) {
    //     stopSounds();
    //     playSoundA(intromusic_data, intromusic_length, 1);
    // }

    state = WIN;
    
}

void lose() {
    if(BUTTON_PRESSED(BUTTON_START)) {
        goToStart();
    }
}

void goToLose() {
    hideSprites();

    REG_BG2HOFF = 0;
    REG_BG2VOFF = 0;

    if(!(REG_DISPCTL & DISP_BACKBUFFER)) {
        flipPages();
    }

    REG_DISPCTL = MODE(4) | BG_ENABLE(2) | DISP_BACKBUFFER;

    drawFullscreenImage4(instructionsBitmap);
    
    waitForVBlank();
    flipPages();
    DMANow(3, instructionsPal, BG_PALETTE, 256);

    // if(state!=INSTRUCTIONS) {
    //     stopSounds();
    //     playSoundA(intromusic_data, intromusic_length, 1);
    // }

    state = LOSE;
}