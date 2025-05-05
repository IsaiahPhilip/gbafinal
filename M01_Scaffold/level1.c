#include "sprites.h"
#include "gba.h"
#include "gameTiles.h"
#include "print.h"
#include "level1.h"
#include "game.h"
#include "mode0.h"
#include "spritesheet.h"
#include "background1.h"
#include "zombiedeath.h"
#include "attack.h"
#include "hurt.h"
#include "supermonkeysfx.h"
#include "superattack.h"
#include "level1collisions.h"
#include "map1.h"
#include "bgsound.h"

typedef enum {IDLE, RUNNING, JUMP, FALL, DASH, ATTACK} ANIMATION_STATES;
typedef enum {DNE, SPAWN, ROAM, DEATH, DEAD} ENEMY_STATE;
typedef enum {R, L} DIRECTION;

inline unsigned char colorAt(int x, int y);
// inline unsigned char exitAt(int x, int y);

void initLevel1() {
    initPlayer1();
    initEnemies1();
}

void initMap1() {
    DMANow(3, gameTilesTiles, &CHARBLOCK[0], gameTilesTilesLen / 2);
    DMANow(3, map1Map, &SCREENBLOCK[27], map1Len / 2);

    for(int i=21; i<32; i++) {
        for(int j=28; j<32; j++) {
            SCREENBLOCK[27].tilemap[OFFSET(i, j, 32)] |= TILEMAP_ENTRY_PALROW(1);
        }
    }
    for(int i=0; i<32; i++) {
        for(int j=28; j<32; j++) {
            SCREENBLOCK[28].tilemap[OFFSET(i, j, 32)] |= TILEMAP_ENTRY_PALROW(1);
        }
    }

    DMANow(3, background1Map, &SCREENBLOCK[29], background1Len / 2);
    DMANow(3, gameTilesPal, BG_PALETTE, 256);

    // the sprite_pal is used by the spritesheet in mode 0
    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITE_PAL, 256);
}

void initEnemies1() {
    initZombies1();
}


// for zombies idle state represents
void initZombies1() {
    for(int i=0; i<TOTALZOMBIES; i++) {
        if(i<3) {
            zombies[i].health = 3;
            zombies[i].numFrames = 8;
            zombies[i].currentFrame = 0;
            zombies[i].state = DNE;
            zombies[i].looking = L;
            zombies[i].timeUntilNextFrame = 10;
            zombies[i].width = 20;
            zombies[i].height = 24;
            zombies[i].idleTick = 0;
            zombies[i].beenhit = 0;
            zombies[i].active = 1;
        } else {
            zombies[i].active = 0;
        }
    }

    zombies[0].x = 240;
    zombies[0].y = 72;
    zombies[0].rightBarrier = 360;
    zombies[0].leftBarrier = 170;

    zombies[1].x = 300;
    zombies[1].rightBarrier = 400;
    zombies[1].leftBarrier = 200;
    zombies[1].y = 72;

    zombies[2].x = 400;
    zombies[2].rightBarrier = 450;
    zombies[2].leftBarrier = 300;
    zombies[2].y = 72;
}

void initPlayer1() {
    mgba_printf("initial y: %d", player.y);
    player.x = 56;
    player.y = SHIFTUP(224);
    player.yVel = SHIFTUP(0);
    player.width = 16;
    player.height = 16;
    player.isAnimating = 0;
    player.currentFrame = 0;
    player.invincibleTick = 0;
    player.state = IDLE;
    player.looking = R;
    player.numFrames = 12;
    player.dashTick = 0;
    player.timeUntilNextFrame = 5;
    player.grounded = 0;
}

void updateLevel1Offsets() {
    hOff = CLAMP(hOff, 0, MAP1WIDTH - SCREENWIDTH);
    vOff = CLAMP(vOff, 0, MAP1HEIGHT - SCREENHEIGHT);

    if(hOff == 0) {
        bghOff = 0;
    } else if(hOff == MAP1WIDTH - SCREENWIDTH) {
        bghOff = (MAP1WIDTH - SCREENWIDTH) / 2;
    }

    if(vOff == MAP1HEIGHT - SCREENHEIGHT) {
        bgvOff = (MAP1HEIGHT - SCREENHEIGHT) / 2;
    } else if(vOff == 0) {
        bgvOff = 0;
    }
}