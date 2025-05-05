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
#include "win3.h"
#include "digitalSound.h"
#include "intromusic.h"
#include "winscreen2.h"
#include "bgsound.h"
#include "winsong2.h"
#include "winsong1.h"
#include "losesong1.h"

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
int startselected;
int pauseselected;

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
    startselected = 0;
    pauseselected = 0;

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
        playSoundB(intromusic_data, intromusic_length, 1);
    }

    if(startselected) {
        BG_PALETTE[32] = BLACK;
        BG_PALETTE[33] = WHITE; 
    } else {
        BG_PALETTE[32] = WHITE;
        BG_PALETTE[33] = BLACK;
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
    // if (BUTTON_PRESSED(BUTTON_SELECT)) {
    //     goToInstructions();
    // }
    mgba_printf("selected %d", startselected);
    if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A) || BUTTON_PRESSED(BUTTON_B)) {
        if(startselected) {
            goToInstructions();
        } else {
            goToGame();
        }
    }
    if(BUTTON_PRESSED(BUTTON_UP) || BUTTON_PRESSED(BUTTON_DOWN)) {
        if(startselected) {
            startselected = 0;
            BG_PALETTE[32] = WHITE;
            BG_PALETTE[33] = BLACK;
        } else {
            startselected = 1;
            BG_PALETTE[32] = BLACK;
            BG_PALETTE[33] = WHITE;
        }
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

    if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A) || BUTTON_PRESSED(BUTTON_B)) {
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

    if(state == PAUSE) {
        REG_DISPCTL = MODE(0) | BG_ENABLE(2) | BG_ENABLE(3) | SPRITE_ENABLE;
        REG_BG2CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(27) | BG_SIZE_WIDE;
        REG_BG3CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(29) | BG_SIZE_WIDE;
        initMap();
    } else {
        stopSounds();
        playSoundB(bgsound_data, bgsound_length, 1);
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
    if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A) || BUTTON_PRESSED(BUTTON_B)) {
        if(pauseselected) {
            goToStart();
        } else {
            goToGame();
        }
    }

    if(BUTTON_PRESSED(BUTTON_UP) || BUTTON_PRESSED(BUTTON_DOWN) || BUTTON_PRESSED(BUTTON_B)) {
        if(pauseselected) {
            pauseselected = 0;
            BG_PALETTE[32] = WHITE;
            BG_PALETTE[33] = BLACK;
        } else {
            pauseselected = 1;
            BG_PALETTE[32] = BLACK;
            BG_PALETTE[33] = WHITE;
        }
    }
}

void goToPause() {
    hideSprites();

    REG_BG2HOFF = 0;
    REG_BG2VOFF = 0;

    if(!(REG_DISPCTL & DISP_BACKBUFFER)) {
        flipPages();
    }

    REG_DISPCTL = MODE(4) | BG_ENABLE(2) | DISP_BACKBUFFER;

    drawFullscreenImage4(pauseBitmap);
    
    waitForVBlank();
    flipPages();
    DMANow(3, pausePal, BG_PALETTE, 256);

    if(pauseselected) {
        BG_PALETTE[32] = BLACK;
        BG_PALETTE[33] = WHITE; 
    } else {
        BG_PALETTE[32] = WHITE;
        BG_PALETTE[33] = BLACK;
    }

    state = PAUSE;
}

void win() {
    if(BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A) || BUTTON_PRESSED(BUTTON_B)) {
        goToStart();
    }
}

void goToWin() {
    hideSprites();
    stopSounds();
    if(score > 4) {
        playSoundB(winsong1_data, winsong1_length, 1);

        REG_BG2HOFF = 0;
        REG_BG2VOFF = 0;

        if(!(REG_DISPCTL & DISP_BACKBUFFER)) {
            flipPages();
        }

        REG_DISPCTL = MODE(4) | BG_ENABLE(2) | DISP_BACKBUFFER;

        drawFullscreenImage4(win3Bitmap);
        
        waitForVBlank();
        flipPages();
        DMANow(3, win3Pal, BG_PALETTE, 256);
    } else {
        playSoundB(winsong2_data, winsong2_length, 1);

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
    }
    

    // if(state!=INSTRUCTIONS) {
    //     stopSounds();
    //     playSoundA(intromusic_data, intromusic_length, 1);
    // }

    state = WIN;
    
}

void lose() {
    if(BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A) || BUTTON_PRESSED(BUTTON_B)) {
        goToStart();
    }
}

void goToLose() {
    hideSprites();
    stopSounds();
    playSoundB(losesong1_data, losesong1_length, 1);

    REG_BG2HOFF = 0;
    REG_BG2VOFF = 0;

    if(!(REG_DISPCTL & DISP_BACKBUFFER)) {
        flipPages();
    }

    REG_DISPCTL = MODE(4) | BG_ENABLE(2) | DISP_BACKBUFFER;

    drawFullscreenImage4(loseBitmap);
    
    waitForVBlank();
    flipPages();
    DMANow(3, losePal, BG_PALETTE, 256);

    // if(state!=INSTRUCTIONS) {
    //     stopSounds();
    //     playSoundA(intromusic_data, intromusic_length, 1);
    // }

    state = LOSE;
}