#include "sprites.h"
#include "gba.h"
#include "gameTiles.h"
#include "print.h"
#include "level3.h"
#include "game.h"
#include "mode0.h"
#include "spritesheet.h"
#include "zombiedeath.h"
#include "attack.h"
#include "hurt.h"
#include "map3.h"
// #include "background3.h"

typedef enum {IDLE, RUNNING, JUMP, FALL, DASH, ATTACK} ANIMATION_STATES;
typedef enum {DNE, SPAWN, ROAM, DEATH, DEAD} ENEMY_STATE;
typedef enum {R, L} DIRECTION;

inline unsigned char colorAt(int x, int y);

void initLevel3() {
    initPlayer3();
    initEnemies3();
}

void initMap3() {
    REG_BG2CNT = 0;
    REG_BG3CNT = 0;
    REG_BG2CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(27) | BG_SIZE_WIDE;
    REG_BG3CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(29) | BG_SIZE_WIDE;

    DMANow(3, gameTilesTiles, &CHARBLOCK[0], gameTilesTilesLen / 2);
    DMANow(3, map3Map, &SCREENBLOCK[27], map3Len / 2);

    // for(int i=0; i<20; i++) {
    //     for(int j=22; j<26; j++) {
    //         SCREENBLOCK[27].tilemap[OFFSET(i, j, 32)] |= TILEMAP_ENTRY_PALROW(1);
    //     }
    // }
    // for(int i=2; i<30; i++) {
    //     for(int j=60-32; j<64-32; j++) {
    //         SCREENBLOCK[28].tilemap[OFFSET(i, j, 32)] |= TILEMAP_ENTRY_PALROW(1);
    //     }
    // }

    // DMANow(3, background3Map, &SCREENBLOCK[29], background3Len / 2);
    DMANow(3, gameTilesPal, BG_PALETTE, 256);

    // the sprite_pal is used by the spritesheet in mode 0
    // DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    // DMANow(3, spritesheetPal, SPRITE_PAL, 256);
}

void updateLevel3Offsets() {
    hOff = CLAMP(hOff, 0, MAP3WIDTH - SCREENWIDTH);
    vOff = CLAMP(vOff, 0, MAP3HEIGHT - SCREENHEIGHT);

    if(hOff == 0) {
        bghOff = 0;
    } else if(hOff == MAP3WIDTH - SCREENWIDTH) {
        bghOff = (MAP3WIDTH - SCREENWIDTH) / 2;
    }

    if(vOff == MAP3HEIGHT - SCREENHEIGHT) {
        bgvOff = (MAP3HEIGHT - SCREENHEIGHT) / 2;
    } else if(vOff == 0) {
        bgvOff = 0;
    }
}

void initPlayer3() {
    mgba_printf("initial y: %d", player.y);
    player.x = 9;
    player.y = SHIFTUP(208);
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
    player.jumpForce = -1500;
}

void initEnemies3() {
    initZombies3();
    initReavers3();
}

// for zombies idle state represents
void initZombies3() {
    for(int i=0; i<3; i++) {
        zombies[i].active = 0;
    }
}

void initReavers3() {
    for(int i=0; i<3; i++) {
        if(i<1) {
            reavers[i].health = 10;
            reavers[i].numFrames = 7;
            reavers[i].currentFrame = 0;
            reavers[i].state = ROAM;
            reavers[i].looking = L;
            reavers[i].timeUntilNextFrame = 10;
            reavers[i].width = 50;
            reavers[i].height = 24;
            reavers[i].idleTick = 0;
            reavers[i].beenhit = 0;
            reavers[i].active = 1;
            reavers[i].isAnimating = 1;
        } else {
            reavers[i].active = 0;
        }

        reavers[0].x = 300;
        reavers[0].y = 208;
        reavers[0].rightBarrier = 426;
        reavers[0].leftBarrier = 20;
    }
}