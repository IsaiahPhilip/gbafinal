#include "sprites.h"
#include "gba.h"
#include "gameTiles.h"
#include "print.h"
#include "level2.h"
#include "game.h"
#include "mode0.h"
#include "spritesheet.h"
#include "zombiedeath.h"
#include "attack.h"
#include "hurt.h"
#include "map2.h"
#include "background2.h"

typedef enum {IDLE, RUNNING, JUMP, FALL, DASH, ATTACK} ANIMATION_STATES;
typedef enum {DNE, SPAWN, ROAM, DEATH, DEAD} ENEMY_STATE;
typedef enum {R, L} DIRECTION;

inline unsigned char colorAt(int x, int y);
// inline unsigned char exitAt(int x, int y);

void initLevel2() {
    initPlayer2();
    initEnemies2();
}

void initMap2() {
    REG_BG2CNT = 0;
    REG_BG3CNT = 0;
    REG_BG2CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(27) | BG_SIZE_TALL;
    REG_BG3CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(29) | BG_SIZE_TALL;

    DMANow(3, gameTilesTiles, &CHARBLOCK[0], gameTilesTilesLen / 2);
    DMANow(3, map2Map, &SCREENBLOCK[27], map2Len / 2);

    for(int i=0; i<20; i++) {
        for(int j=22; j<26; j++) {
            SCREENBLOCK[27].tilemap[OFFSET(i, j, 32)] |= TILEMAP_ENTRY_PALROW(1);
        }
    }
    for(int i=2; i<30; i++) {
        for(int j=60-32; j<64-32; j++) {
            SCREENBLOCK[28].tilemap[OFFSET(i, j, 32)] |= TILEMAP_ENTRY_PALROW(1);
        }
    }

    DMANow(3, background2Map, &SCREENBLOCK[29], background2Len / 2);
    DMANow(3, gameTilesPal, BG_PALETTE, 256);

    // the sprite_pal is used by the spritesheet in mode 0
    // DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    // DMANow(3, spritesheetPal, SPRITE_PAL, 256);
}

void updateLevel2Offsets() {
    hOff = CLAMP(hOff, 0, MAP2WIDTH - SCREENWIDTH);
    vOff = CLAMP(vOff, 0, MAP2HEIGHT - SCREENHEIGHT);

    if(hOff == 0) {
        bghOff = 0;
    } else if(hOff == MAP2WIDTH - SCREENWIDTH) {
        // mgba_printf("hOff:%d", hOff);
        // mgba_printf("bghOff:%d", bghOff);
        bghOff = (MAP2WIDTH - SCREENWIDTH) / 2;
    }
    if(vOff == 0) {
        bgvOff = 0;
    } else if(vOff == MAP2HEIGHT - SCREENHEIGHT) {
        bgvOff = (MAP2HEIGHT - SCREENHEIGHT) / 2;
    }
}

void initPlayer2() {
    mgba_printf("initial y: %d", player.y);
    player.x = 32;
    player.y = SHIFTUP(48);
    player.yVel = SHIFTUP(0);
    player.width = 16;
    player.height = 16;
    player.isAnimating = 0;
    player.currentFrame = 0;
    player.state = IDLE;
    player.looking = R;
    player.numFrames = 12;
    player.dashTick = 0;
    player.timeUntilNextFrame = 5;
    player.grounded = 0;
}

void initEnemies2() {
    initZombies2();
}

// for zombies idle state represents
void initZombies2() {
    for(int i=0; i<TOTALZOMBIES; i++) {
        if(i<1) {
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

    zombies[0].x = 100;
    zombies[0].y = 432;
    zombies[0].rightBarrier = 150;
    zombies[0].leftBarrier = 90;
}