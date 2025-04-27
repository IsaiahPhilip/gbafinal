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

// now I've gotta add another level
// I think the best way to go about this is to mark every function that needs to be redone
// For functions that can be global I might need to pass in the current level
// There are three kinds of functions then
// Functions specific to the level (A)
// Global functions that require the current level to be passed in 2 (B)
// and global functions that do not require any attributes (C)

//A
typedef enum {IDLE, RUNNING, JUMP, FALL, DASH, ATTACK} ANIMATION_STATES;
typedef enum {DNE, SPAWN, ROAM, DEATH, DEAD} ENEMY_STATE;
typedef enum {R, L} DIRECTION;

ENEMY zombies[3];

// int jumping = 0;
// int dashTick = 0;

inline unsigned char colorAt(int x, int y);
// inline unsigned char exitAt(int x, int y);

// now I'm going to implement the enemy:
// in initialize its initial position and state should be set
// when enemy.x - player.x is a specific value, the spawn animation should be triggered
// once the spawn completes the enemy should roam from right to left
// additional: if the enemy is facing the player and the player is within a certain range the enemy will move towards the player and its movement speed will increase
// states: spawn, roam, chase, death

// B, can be refactored into generic initLevel(int currentLevel)
void initLevel1() {
    
    initPlayer1();
    initEnemies1();

    hideSprites();
    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128*4);
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
    for(int i=0; i<3; i++) {
        zombies[i].health = 3;
        zombies[i].numFrames = 8;
        zombies[i].currentFrame = 0;
        zombies[i].state = DNE;
        zombies[i].looking = L;
        zombies[i].timeUntilNextFrame = 10;
        zombies[i].width = 20;
        zombies[i].height = 24;
        zombies[i].oamIndex = i+1;
        zombies[i].beenhit = 0;
    }

    zombies[0].x = 240;
    zombies[0].y = 72;
    zombies[0].rightBarrier = 360;
    zombies[0].leftBarrier = 120;

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
    player.jumpForce = -1500;
}

void updateLevel1() {
    updatePlayer1();
    updateEnemies1();

    hOff = player.x - (SCREENWIDTH - player.width) / 2;
    vOff = SHIFTDOWN(player.y) - (SCREENHEIGHT - player.height) / 2;
    bghOff = ((player.x / 2) - (SCREENWIDTH - player.width) / 2) + 58;
    bgvOff = ((SHIFTDOWN(player.y) / 2) - (SCREENHEIGHT - player.height) / 2);

    hOff = CLAMP(hOff, 0, MAP1WIDTH - SCREENWIDTH);
    vOff = CLAMP(vOff, 0, MAP1HEIGHT - SCREENHEIGHT);

    if(hOff == 0) {
        bghOff = 0;
    } else if(hOff == 272) {
        // mgba_printf("hOff:%d", hOff);
        // mgba_printf("bghOff:%d", bghOff);
        bghOff = 138;
    }

    if(vOff == 0) {
        bgvOff = 0;
    }

    // mgba_printf("bgvOff %d", bgvOff);
    // mgba_printf("vOff:%d", hOff);

    // bghOff = CLAMP(bghOff, hOff, 100000);
}

void updateEnemies1() {
   updateZombies();
}

void updateZombies() {
    // for each zombie
   // enemies will be handled through a switch statement based on their 'state'
   // the DNE state is the initial default state for every enemy with a spawn animation
   // 
    // store the initial left and right barriers for roaming within member variables
    // based on the value of the looking variable, check if the zombie has reached the bound
    //    if so flip zombie

    // within DNE we should only check for when player is close enough for spawn to begin

    for(int i=0; i<3; i++) {
        if(zombies[i].state == DNE) {
            if(zombies[i].x - 100 < player.x) {
                // the frame variables for spawn should also be set here
                // isAnimating should be set to 1 and stay at one until the animation is over
                // numFrames should be set to 8
                // timeBetweenFrames should be set to 10
                // in the animation handler, rather that using modulo to loop, 
                //     once currentFrame == numFrames we should switch to the roam state
                zombies[i].state = SPAWN;
                zombies[i].isAnimating = 1;
                zombies[i].numFrames = 8;
                zombies[i].timeUntilNextFrame = 10;
            }
        } else if(zombies[i].state == SPAWN) {
            // zombies can take damage in this state thats about it for functionality
            // once the animation ends the zombie should transition to the roaming state.
    
        } else if(zombies[i].state == ROAM) {
            if(zombies[i].looking == L) {
                zombies[i].x--;
                if(zombies[i].x < zombies[i].leftBarrier) {
                    zombies[i].looking = R;
                }
            } else {
                zombies[i].x++;
                if(zombies[i].x > zombies[i].rightBarrier) {
                    zombies[i].looking = L;
                }
            }
        }
        // need to handle death state
        // once the death state is triggered the animation should play and once the animation completes we will hide the zombie
    
        // different types of animation handling needed:
        // looping, loops indefiniteley, we have this working
        // finite, ends in a single run through, need to add this functionality
    
        if(zombies[i].state == SPAWN) {
            // mgba_printf("state!!!!");
            --zombies[i].timeUntilNextFrame;
            if(zombies[i].timeUntilNextFrame == 0) {
                zombies[i].currentFrame++;
                if(zombies[i].currentFrame >= zombies[i].numFrames - 1) {
                    zombies[i].state = ROAM;
                }
                zombies[i].timeUntilNextFrame = 15;
            }
        } else if(zombies[i].state == DEATH) {
            --zombies[i].timeUntilNextFrame;
            if(zombies[i].timeUntilNextFrame == 0) {
                zombies[i].currentFrame++;
                if(zombies[i].currentFrame >= zombies[i].numFrames - 1) {
                    zombies[i].state = DEAD;
                    shadowOAM[zombies[i].oamIndex].attr0 = ATTR0_HIDE;
                }
                zombies[i].timeUntilNextFrame = 10;
            }
        } else {
            if(zombies[i].isAnimating) {
                --zombies[i].timeUntilNextFrame;
                if(zombies[i].timeUntilNextFrame == 0) {
                    zombies[i].currentFrame = ((zombies[i].currentFrame + 1)) % (zombies[i].numFrames);
                    zombies[i].timeUntilNextFrame = 15;
                }
            } else {
                zombies[i].currentFrame = 0;
                zombies[i].timeUntilNextFrame = 10;
            }
        }
    }
}

// will handle collision checks and handling for the enemy that is passed in
void enemyDamage() {
    // if the player collides with enemy the player should flash red to indicate taking damage, then white to indicate short invincibility.
    if(player.invincibleTick == 0) {
        for(int i=0; i<3; i++) {
            if(zombies[i].state != DEATH && zombies[i].state != DEAD) {
                // play player damaged sound
                // playAnalogueSound()
                if(zombies[i].looking == L) {
                    if(player.invincibleTick == 0 && collision(zombies[i].x, zombies[i].y, zombies[i].width, zombies[i].height, player.x, SHIFTDOWN(player.y), player.width, player.height)) {
                        player.health--;
                        player.invincibleTick = 30;
                        shadowOAM[hearts[player.health].oamIndex].attr0 |= ATTR0_HIDE;
                        hearts[player.health].active = 0;
                    }
                } else {
                    if(player.invincibleTick == 0 && collision(zombies[i].x + 8, zombies[i].y, zombies[i].width, zombies[i].height, player.x, SHIFTDOWN(player.y), player.width, player.height)) {
                        player.health--;
                        player.invincibleTick = 30;
                        shadowOAM[hearts[player.health].oamIndex].attr0 |= ATTR0_HIDE;
                        hearts[player.health].active = 0;
                    }
                }
            }
        }
    }
}

void updatePlayer1() {
    player.isAnimating = 0;
    int leftX = player.x;
    int rightX = player.x + player.width - 1;
    int topY = SHIFTDOWN(player.y);
    int bottomY = SHIFTDOWN(player.y) + player.height;

    if(player.invincibleTick) {
        player.invincibleTick--;
    } else {
        enemyDamage();
    }

    if(attackCooldown) {
        attackCooldown--;
    }

    if(player.dashTick) {
        player.dashTick--;
    }

    player.state = IDLE;

    if(!player.grounded) {
        if(player.yVel > 0) {
            player.state = FALL;
        } else {
            player.state = JUMP;
        }
    }
    // mgba_printf("color: %d", colorAt(leftX, topY));
    if (BUTTON_PRESSED(BUTTON_RSHOULDER) && BUTTON_PRESSED(BUTTON_LSHOULDER)) {
        if(player.isCheating) {
            player.isCheating = 0;
        } else {
            playSoundB(supermonkeysfx_data, supermonkeysfx_length, 0);
            player.isCheating = 1;
        }
    }
    if (BUTTON_HELD(BUTTON_LEFT) && (colorAt(leftX, topY) && colorAt(leftX, bottomY - 1))) {
        player.xVel = -2;
        // if there is an incoming collison
        if(!(colorAt(leftX + player.xVel, bottomY - 1) && colorAt(leftX + player.xVel, topY - 1))) {
            player.xVel = -1;
        }
        player.isAnimating = 1;
        if(player.state == JUMP || player.state == FALL) {
            // player.xVel++;
        } else {
            player.state = RUNNING;
        }
        player.looking = L;
    } else if(BUTTON_HELD(BUTTON_RIGHT) && (colorAt(rightX, topY) && colorAt(rightX, bottomY - 1))) {
        player.xVel = 2;
        if(!(colorAt(rightX + player.xVel, bottomY-1) && colorAt(rightX + player.xVel, topY-1))) {
            player.xVel = 1;
        }
        player.isAnimating = 1;
        if(player.state == JUMP || player.state == FALL) {
            // player.xVel--;
        } else {
            player.state = RUNNING;
        }
        player.looking = R;
    }

    // if(colorAt(leftX, topY) && colorAt(leftX, bottomY - 1))
    // if(BUTTON_PRESSED(BUTTON_A) && player.dashTick==0) {
    //     player.xVel*=20;
    //     player.dashTick = 60;
    // }
    playerAttack();
    if (BUTTON_PRESSED(BUTTON_UP) && player.grounded) {
        player.state = JUMP;
        player.grounded = 0;
        player.yVel = player.jumpForce;
    }

    player.grounded = !(colorAt(leftX + 1, bottomY) && colorAt(rightX - 1, bottomY));
    mgba_printf("grounded: %d", player.grounded);
    // mgba_printf("color left: %d", colorAt(leftX, bottomY));
    // mgba_printf("color right: %d", colorAt(rightX, bottomY));

    if(!player.grounded) {
        player.yVel = CEILING(player.yVel + GRAVITY, TERMINALVELOCITY);
        // head bonk
        if(!(colorAt(leftX + 1, topY + SHIFTDOWN(player.yVel)) && colorAt(rightX - 1, topY + SHIFTDOWN(player.yVel)))) {
            player.yVel = 0;
        }
        if(player.state == JUMP && player.yVel > 0) {
            // mgba_printf("max jump height: %d", SHIFTDOWN(player.y));
            player.state = FALL;
        }
        if(!(colorAt(leftX + 1, bottomY + SHIFTDOWN(player.yVel)) && colorAt(rightX - 1, bottomY + SHIFTDOWN(player.yVel)))) { // if there is floor below
            // tried fall damage, but fixed point makes it kind impossible without dramatically bumping terminal velocity up which would make the falls, and thereby vOff, kind violent
            // if(player.yVel > DAMAGEVELOCITY) {
            //     player.health--;
            //     player.invincibleTick = 30;
            //     shadowOAM[hearts[player.health].oamIndex].attr0 |= ATTR0_HIDE;
            //     hearts[player.health].active = 0;
            // }
            // player.grounded = 1;
            // player.state = IDLE;
            // mgba_printf("AHHHHHHH %d", player.grounded);
            player.yVel = SHIFTUP(1);
        }
        mgba_printf("yVel %d", player.yVel);
    } else {
        
        player.yVel = CEILING(player.yVel, 0);
    }

    // mgba_printf("y velocity: %d", SHIFTDOWN(player.yVel));

    player.y+=player.yVel;

    // trigger death if player falls off level
    if(SHIFTDOWN(player.y) > 256) {
        player.health = 0;
    }
    
    // if there is a collision
    player.x+=player.xVel;
    if(player.xVel > 0) {
        player.xVel--;
    } else if(player.xVel < 0) {
        player.xVel++;
    }

    // if(player.x + player.width > MAPWIDTH) {
    //     player.x = MAPWIDTH - player.width;
    // } else if(player.x < 0) {
    //     player.x == 0;
    // }
    
    // if collision is occuring at the bottom revert the change or disable if bottom is colliding prior to move
    if(player.isAnimating) {
        --player.timeUntilNextFrame;
        if(player.timeUntilNextFrame == 0) {
            player.currentFrame = ((player.currentFrame + 1)) % (player.numFrames);
            player.timeUntilNextFrame = 5;
        }
    } else {
        player.currentFrame = 0;
        player.timeUntilNextFrame = 5;
    }

    if(slash.isAnimating) {
        slashEnemy();
        --slash.timeUntilNextFrame;
        if(slash.timeUntilNextFrame == 0) {
            slash.currentFrame++;
            if(slash.currentFrame == slash.numFrames - 1) {
                slash.isAnimating = 0;
                shadowOAM[slash.oamIndex].attr0 = ATTR0_HIDE;
                for(int i=0; i<3; i++) {
                    zombies[i].beenhit = 0;
                }
            }
            slash.timeUntilNextFrame = 5;
        }
    } else {
        slash.currentFrame = 0;
        slash.timeUntilNextFrame = 5;
    }
    
    // mgba_printf("current frame: %d", player.currentFrame);
    // mgba_printf("playerx:%d", player.x);
    // mgba_printf("current level: %d", currentLevel);
}

void playerAttack() {
    // need to implement attack functionality
    // attack will have a range of 32 x 16 in the direction the player is facing
    // attack particles will be triggered by press and play once
    // if player is looking right particle.x = player.x + player.width
    // if player is looking left particle.x = player.x - player.width - particle.width (hflip)
    if (BUTTON_PRESSED(BUTTON_B) && attackCooldown == 0 && (player.state != ATTACK && player.state != DASH)) {
        // player.priorState = player.state;
        // player.state = ATTACK;
        if(player.isCheating) {
            playSoundA(superattack_data, superattack_length, 0);
        } else {
            playSoundA(attack_data, attack_length, 0);
        }
        
        if(player.isCheating) {
            attackCooldown = 10;
        } else {
            attackCooldown = 30;
        }
        slash.isAnimating = 1;
        slash.currentFrame = 0;
        if(player.looking == R) {
            slash.x = player.x + player.width;
        } else {
            slash.x = player.x - slash.width;
        }
        slash.y = SHIFTDOWN(player.y);
        slash.direction = player.looking;
    }
}

void slashEnemy() {
    for(int i=0; i<3; i++) {
        if(zombies[i].state != DEAD && slash.isAnimating && !zombies[i].beenhit && collision(zombies[i].x, zombies[i].y, zombies[i].width, zombies[i].height, slash.x, slash.y, slash.width, slash.height)) {
            mgba_printf("zombie data: %d", zombies[i].state);
            playSoundB(hurt_data, hurt_length, 0);
            if(player.isCheating) {
                zombies[i].health -= 2;
            } else {
                zombies[i].health--;
            }
            zombies[i].beenhit = 1;
            if(zombies[i].health <= 0) {
                zombies[i].health = 0;
                // need to access tiles below current x, y pos
                // then change the tileID to the bloodied version
                // sets the tile under the zombie to the tile in the tileset under the current one.
                // mgba_printf("zombies: %d %d", zombies[i].x, zombies[i].y);

                if(zombies[i].x < 240) {
                    SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8 + 3, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8 + 2, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8 + 1, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                } else if(zombies[i].x > 256){
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8), (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 3, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 2, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 1, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                }
                zombies[i].state = DEATH;
                zombies[i].currentFrame = 0;
                zombies[i].numFrames = 8;
                zombies[i].timeUntilNextFrame = 10;
            }
        }
    }
}

void drawLevel1() {
    drawPlayer();
    drawEnemies1();
    drawUI();

    if(player.health==0) {
        // I think for now having the player be sent to a lose screen makes sense since it isn't that difficult 
        // and there aren't any instadeaths since the player can't fall off my current map 1
        hOff = 0;
        vOff = 0;
        goToLose();
    }
    // mgba_printf("color: %d", colorAt(player.x + 8, player.y ));


    if(colorAt(player.x + 8, SHIFTDOWN(player.y) + 8) == 2) {
        hOff = 0;
        vOff = 0;
        // 1. how will I be handling the actual win condition for each level -> The player will have to enter the door fully
        // 2. gotta change this from go to win to go to next level
        currentLevel++;
        initNext();
        // goToWin();
    }

    REG_BG2HOFF = hOff;
    REG_BG2VOFF = vOff;
    REG_BG3HOFF = bghOff;
    // REG_BG3VOFF = bgvOff;

    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128*4);
}

void drawPlayer() {
    shadowOAM[player.oamIndex].attr0 = ATTR0_Y(SHIFTDOWN(player.y) - vOff + 1) | ATTR0_SQUARE;

    if(colorAt(player.x + 8, SHIFTDOWN(player.y) + 8) == 2 || colorAt(player.x + 8, SHIFTDOWN(player.y) + 8) == 3) {
        shadowOAM[player.oamIndex].attr0 |= ATTR0_HIDE;
    }

    shadowOAM[player.oamIndex].attr1 = ATTR1_X(player.x - hOff) | ATTR1_SMALL;
    int playerPal = 0;
    if(player.isCheating) {
        playerPal = 3;
    }
    if(player.invincibleTick > 0) {
        if(player.invincibleTick > 20) {
            playerPal = 1;
        } else {
            playerPal = 2;
        }
    }
    if (player.looking) {
        shadowOAM[player.oamIndex].attr1 |= ATTR1_HFLIP;
    }
    if (player.state == JUMP || player.state == FALL) {
        shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(playerPal) | ATTR2_TILEID(0, 0);
    } 
    // else if(player.state == ATTACK) { 
    //     shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(0, 0);
    // }
    else {
        shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(playerPal) | ATTR2_TILEID(2 * player.currentFrame, 2 * player.state);
    }

    // mgba_printf("current frame: %d", slash.currentFrame);
    // mgba_printf("frame count: %d", slash.numFrames);

    if(slash.isAnimating) {
        shadowOAM[slash.oamIndex].attr0 = ATTR0_Y(slash.y - vOff) | ATTR0_WIDE;
        shadowOAM[slash.oamIndex].attr1 = ATTR1_X(slash.x - hOff) | ATTR1_MEDIUM;
        if (slash.direction) {
            shadowOAM[slash.oamIndex].attr1 |= ATTR1_HFLIP;
        }
        shadowOAM[slash.oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4 * slash.currentFrame, 8);
    }
      
}

drawEnemies1() {
    for(int i=0; i<3; i++) {
        // mgba_printf("enemy: %d, current: %d, state: %d", i+1, zombies[i].currentFrame, zombies[i].state);
        if(zombies[i].state != DEAD) {
            shadowOAM[zombies[i].oamIndex].attr0 = ATTR0_Y(zombies[i].y - vOff) | ATTR0_SQUARE;
            shadowOAM[zombies[i].oamIndex].attr1 = ATTR1_X(zombies[i].x - hOff) | ATTR1_MEDIUM;
            if(zombies[i].state == DNE) {
                shadowOAM[zombies[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(0, 20);
            } else if(zombies[i].state == SPAWN || zombies[i].state == ROAM || zombies[i].state == DEATH) {
                if(zombies[i].beenhit) {
                    shadowOAM[zombies[i].oamIndex].attr2 = ATTR2_PALROW(2) | ATTR2_TILEID(zombies[i].currentFrame * 4, 20 + ((zombies[i].state - 1)*4));
                } else {
                    shadowOAM[zombies[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(zombies[i].currentFrame * 4, 20 + ((zombies[i].state - 1)*4));
                }
            }
            if(!zombies[i].looking) {
                shadowOAM[zombies[i].oamIndex].attr1 |= ATTR1_HFLIP; 
            } 
        }
    }
}

void drawUI() {
    for(int i=0; i<3; i++) {
        // mgba_printf("heart info: %d", hearts[i].x);
        if(hearts[i].active) {
            shadowOAM[hearts[i].oamIndex].attr0 = ATTR0_Y(hearts[i].y) | ATTR0_SQUARE;
            shadowOAM[hearts[i].oamIndex].attr1 = ATTR1_X(hearts[i].x) | ATTR1_SMALL;
            shadowOAM[hearts[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(16, 0);
        }
    }
}

inline unsigned char colorAt(int x, int y){
    return ((unsigned char *) level1collisionsBitmap) [OFFSET(x, y, MAP1WIDTH)];
}