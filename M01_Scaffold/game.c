#include "gba.h"
#include "print.h"
#include "game.h"
#include "mode0.h"
#include "level1.h"

int score;
int currentLevel;
int attackCooldown;

int vOff;
int hOff;
int bghOff;
int bgvOff;
PLAYER player;
OBJECT hearts[3];
SPRITE slash;
OBJ_ATTR shadowOAM[128];
typedef enum {DOWN, RIGHT, UP, LEFT} DIRECTION;

void initGame() {
    // mgba_printf("there");
    hOff = 0;
    vOff = 0;
    bghOff = 0;
    bgvOff = 0;
    score = 0;
    currentLevel = 1;
    attackCooldown = 0;

    player.health = 3; 
    player.oamIndex = 0;
    player.isCheating = 0;

    for(int i=0; i<player.health; i++) {
        hearts[i].x = i * 20;
        hearts[i].y = 2;
        hearts[i].active = 1;
        hearts[i].oamIndex = i + 5;
    }
    
    slash.width = 32;
    slash.height = 16;
    slash.numFrames = 4;
    slash.oamIndex = 8;
    slash.isAnimating = 0;
    slash.timeUntilNextFrame = 5;
}
// instead I want initMap to handle the initialization stuff here
// t
void initMap() {
    if(currentLevel == 1) {
        initMap1();
    }
}

void initLevel() {
    if(currentLevel == 1) {
        initLevel1();
    }
}

void updateLevel() {
    if(currentLevel == 1) {
        updateLevel1();
    }
}

void drawLevel() {
    if(currentLevel == 1) {
        drawLevel1();
    }
}

void initNext() {
    if(currentLevel > LASTLEVEL) {
        goToWin();
    } else {
        initLevel();
        initMap();
    }
}