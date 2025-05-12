#include "gba.h"
#include "print.h"
#include "game.h"
#include "mode0.h"
#include "level1.h"
#include "level2.h"
#include "level3.h"
#include "level1collisions.h"
#include "level2collisions.h"
#include "level3collisions.h"
#include "supermonkeysfx.h"
#include "reaverscream.h"
#include "superattack.h"
#include "attack.h"
#include "hurt.h"

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
ENEMY zombies[TOTALZOMBIES];
ENEMY reavers[TOTALREAVERS];

inline unsigned char colorAt(int x, int y);

// typedef enum {DOWN, RIGHT, UP, LEFT} DIRECTION;

typedef enum {IDLE, RUNNING, JUMP, FALL, DASH, ATTACK} ANIMATION_STATES;
typedef enum {DNE, SPAWN, ROAM, DEATH, DEAD, CHASE} ENEMY_STATE;
typedef enum {R, L} DIRECTION;

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
    player.jumpForce = -1800;

    for(int i=0; i<player.health; i++) {
        hearts[i].x = i * 20;
        hearts[i].y = 2;
        hearts[i].active = 1;
        hearts[i].oamIndex = i + 11;
    }

    for(int i=0; i<TOTALZOMBIES; i++) { 
        zombies[i].oamIndex = i+1;
        zombies[i].active = 0;
    }
    
    slash.width = 32;
    slash.height = 16;
    slash.numFrames = 4;
    slash.oamIndex = 14;
    slash.isAnimating = 0;
    slash.timeUntilNextFrame = 5;

    for(int i=0; i<TOTALREAVERS; i++) { 
        reavers[i].oamIndex = i+15;
        reavers[i].active = 0;
    }
}

// instead I want initMap to handle the initialization stuff here
// t
void initMap() {
    if(currentLevel == 1) {
        initMap1();
    } else if(currentLevel == 2) {
        initMap2();
    } else if(currentLevel == 3) {
        initMap3();
    }
}

void initLevel() {
    if(currentLevel == 1) {
        initLevel1();
    } else if(currentLevel == 2) {
        initLevel2();
    } else if(currentLevel == 3) {
        initLevel3();
    }

    hideSprites();
    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128*4);
}

void updatePlayer() {
    // mgba_printf("player x %d", player.x);
    // mgba_printf("player y: %d", SHIFTDOWN(player.y));
    player.isAnimating = 0;
    int leftX = player.x;
    int rightX = player.x + player.width - 1;
    int topY = SHIFTDOWN(player.y);
    int bottomY = SHIFTDOWN(player.y) + player.height;

    if(player.invincibleTick) {
        player.invincibleTick--;
    } else {
        enemyDamage();
        levelDamage();
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
            playSoundA(supermonkeysfx_data, supermonkeysfx_length, 0);
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
    if ((BUTTON_PRESSED(BUTTON_UP) || BUTTON_PRESSED(BUTTON_A)) && player.grounded) {
        player.state = JUMP;
        player.grounded = 0;
        player.yVel = player.jumpForce;
    }

    player.grounded = !(colorAt(leftX + 1, bottomY) && colorAt(rightX - 1, bottomY));
    // mgba_printf("grounded: %d", player.grounded);

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
        // mgba_printf("yVel %d", player.yVel);
    } else {
        
        player.yVel = CEILING(player.yVel, 0);
    }

    // mgba_printf("y velocity: %d", SHIFTDOWN(player.yVel));

    player.y+=player.yVel;

    // trigger death if player falls off level
    // if(SHIFTDOWN(player.y) > 256) {
    //     player.health = 0;
    // }
    
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
                for(int i=0; i<TOTALREAVERS; i++) {
                    reavers[i].beenhit = 0;
                }
            }
            slash.timeUntilNextFrame = 5;
        }
    } else {
        slash.currentFrame = 0;
        slash.timeUntilNextFrame = 5;
    }
}

void slashEnemy() {
    for(int i=0; i<TOTALZOMBIES; i++) {
        if(zombies[i].active && zombies[i].state != DEAD && slash.isAnimating && !zombies[i].beenhit && collision(zombies[i].x, zombies[i].y, zombies[i].width, zombies[i].height, slash.x, slash.y, slash.width, slash.height)) {
            // mgba_printf("zombie data: %d", zombies[i].state);
            // playSoundB(hurt_data, hurt_length, 0);
            if(player.isCheating) {
                zombies[i].health -= 3;
            } else {
                zombies[i].health--;
            }
            zombies[i].beenhit = 1;
            if(zombies[i].health <= 0) {
                score++;
                zombies[i].health = 0;
                // need to access tiles below current x, y pos
                // then change the tileID to the bloodied version
                // sets the tile under the zombie to the tile in the tileset under the current one.
                // mgba_printf("zombies: %d %d", zombies[i].x, zombies[i].y);
                if(currentLevel == 1) {
                    if(zombies[i].x < 232) {
                        SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8 + 3, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8 + 2, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8 + 1, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        SCREENBLOCK[27].tilemap[OFFSET(zombies[i].x / 8, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    } else if(zombies[i].x > 256){
                        SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8), (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 3, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 2, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 1, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                    } else {
                        if(zombies[i].x < 240) {
                            SCREENBLOCK[28].tilemap[OFFSET(0, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[27].tilemap[OFFSET(31, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[27].tilemap[OFFSET(30, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[27].tilemap[OFFSET(29, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        } else if(zombies[i].x < 248) {
                            SCREENBLOCK[27].tilemap[OFFSET(31, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[27].tilemap[OFFSET(30, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[28].tilemap[OFFSET(0, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[28].tilemap[OFFSET(1, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        } else {
                            SCREENBLOCK[27].tilemap[OFFSET(31, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[28].tilemap[OFFSET(0, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[28].tilemap[OFFSET(1, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                            SCREENBLOCK[28].tilemap[OFFSET(2, (zombies[i].y / 8) + 4, 32)] = TILEMAP_ENTRY_TILEID(45);
                        }
                    }
                }
                if(currentLevel == 2) {
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8), ((zombies[i].y - 256) / 8) + 5, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 3, ((zombies[i].y - 256) / 8) + 5, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 2, ((zombies[i].y - 256) / 8) + 5, 32)] = TILEMAP_ENTRY_TILEID(45);
                    SCREENBLOCK[28].tilemap[OFFSET(((zombies[i].x - 256) / 8) + 1, ((zombies[i].y - 256) / 8) + 5, 32)] = TILEMAP_ENTRY_TILEID(45);
                }

                zombies[i].state = DEATH;
                zombies[i].currentFrame = 0;
                zombies[i].numFrames = 8;
                zombies[i].timeUntilNextFrame = 10;
            }
        }
    }
    for(int i=0; i<TOTALREAVERS; i++) {
        if(reavers[i].active && reavers[i].state != DEAD && slash.isAnimating && !reavers[i].beenhit && collision(reavers[i].x, reavers[i].y, reavers[i].width, reavers[i].height, slash.x, slash.y, slash.width, slash.height)) {
            if(player.isCheating) {
                reavers[i].health -= 3;
            } else {
                reavers[i].health--;
            }
            reavers[i].beenhit = 1;
            if(reavers[i].health <= 0) {
                score++;
                reavers[i].health = 0;
                reavers[i].state = DEATH;
                reavers[i].currentFrame = 0;
                reavers[i].numFrames = 1;
                reavers[i].timeUntilNextFrame = 60;
            }
        }
    }
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

void updateEnemies() {
    updateZombies();
    updateReavers();
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
 
     for(int i=0; i<TOTALZOMBIES; i++) {
        if(zombies[i].active) {
            if(zombies[i].state == DNE) {
                if(zombies[i].x - 100 < player.x) {
                    zombies[i].state = SPAWN;
                    zombies[i].isAnimating = 1;
                    zombies[i].numFrames = 8;
                    zombies[i].timeUntilNextFrame = 10;
                }
            } else if(zombies[i].state == SPAWN) {
                // zombies can take damage in this state thats about it for functionality
                // once the animation ends the zombie should transition to the roaming state.
        
            } else if(zombies[i].state == ROAM) {
                if(!zombies[i].idleTick) {
                    if(zombies[i].looking == L) {
                        if(!zombies[i].beenhit) {
                            zombies[i].x--;
                        }
                        if(zombies[i].x < zombies[i].leftBarrier) {
                            // zombies[i].looking = R;
                            zombies[i].idleTick = 100;
                            zombies[i].isAnimating = 0;
                        }
                    } else {
                        if(!zombies[i].beenhit) {
                            zombies[i].x++;
                        }
                        if(zombies[i].x > zombies[i].rightBarrier) {
                            // zombies[i].looking = L;
                            zombies[i].idleTick = 100;
                            zombies[i].isAnimating = 0;
                        }
                    }
                } else {
                    zombies[i].idleTick--;
                    if(!zombies[i].idleTick) {
                        zombies[i].isAnimating = 1;
                        if(zombies[i].looking) {
                            zombies[i].looking = 0;
                        } else {
                            zombies[i].looking = 1;
                        }
                    }
                }
            }
        
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
 }

 void updateReavers() {
    // states: roam, chase, death, dead
    for(int i=0; i<TOTALREAVERS; i++) {
        if(reavers[i].active) {
            if(reavers[i].state == ROAM) {
                if(!reavers[i].idleTick) {
                    if(reavers[i].looking == L) {
                        reavers[i].x--;
                        if(reavers[i].x < reavers[i].leftBarrier) {
                            // zombies[i].looking = R;
                            reavers[i].idleTick = 100;
                            reavers[i].isAnimating = 0;
                        }
                    } else {
                        reavers[i].x++;
                        if(reavers[i].x > reavers[i].rightBarrier) {
                            // zombies[i].looking = L;
                            reavers[i].idleTick = 100;
                            reavers[i].isAnimating = 0;
                        }
                    }
                } else {
                    reavers[i].idleTick--;
                    if(!reavers[i].idleTick) {
                        reavers[i].isAnimating = 1;
                        if(reavers[i].looking) {
                            reavers[i].looking = 0;
                        } else {
                            reavers[i].looking = 1;
                        }
                    }
                }
                if(reavers[i].y - SHIFTDOWN(player.y) < 10 && abs(reavers[i].x - player.x) < 100) {
                    reavers[i].state = CHASE;
                    reavers[i].speedTick = 50;
                    playSoundB(reaverscream_data, reaverscream_length, 0);
                }
            }

            if(reavers[i].state == CHASE) {
                int xDist = (reavers[i].x+32) - player.x;
                reavers[i].speedTick--;
                if(reavers[i].speedTick == 0) {
                    reavers[i].speedTick = 20;
                    if(xDist < 0) { // reaver is to the right of player
                        reavers[i].xVel++;
                    } else {
                        reavers[i].xVel--;
                    }
                }

                reavers[i].xVel = CLAMP(reavers[i].xVel, -3, 3);
                reavers[i].x = CLAMP(reavers[i].xVel + reavers[i].x, reavers[i].leftBarrier, reavers[i].rightBarrier);
                if(reavers[i].xVel > 0) {
                    reavers[i].looking = 0;
                } else {
                    reavers[i].looking = 1;
                }

            }

            mgba_printf("reaver state %d", reavers[i].state);
        
            if(reavers[i].state == DEATH) {
                --reavers[i].timeUntilNextFrame;
                if(reavers[i].timeUntilNextFrame == 0) {
                    reavers[i].currentFrame++;
                    if(reavers[i].currentFrame >= reavers[i].numFrames - 1) {
                        reavers[i].state = DEAD;
                        shadowOAM[reavers[i].oamIndex].attr0 = ATTR0_HIDE;
                    }
                    reavers[i].timeUntilNextFrame = 10;
                }
            } else {
                if(reavers[i].isAnimating) {
                    --reavers[i].timeUntilNextFrame;
                    if(reavers[i].timeUntilNextFrame == 0) {
                        reavers[i].currentFrame = ((reavers[i].currentFrame + 1)) % (reavers[i].numFrames);
                        reavers[i].timeUntilNextFrame = 15;
                    }
                } else {
                    reavers[i].currentFrame = 0;
                    reavers[i].timeUntilNextFrame = 10;
                }
            }
        }
     }
 }

 void levelDamage() {
    if(colorAt(player.x + 8, SHIFTDOWN(player.y) + 8) == 4) {
        if(!player.invincibleTick) {
            player.health--;
            player.invincibleTick = 30;
            shadowOAM[hearts[player.health].oamIndex].attr0 |= ATTR0_HIDE;
            hearts[player.health].active = 0;
        }
    }
 }

 void enemyDamage() {
    // if the player collides with enemy the player should flash red to indicate taking damage, then white to indicate short invincibility.
    if(player.invincibleTick == 0) {
        for(int i=0; i<TOTALZOMBIES; i++) {
            if(zombies[i].active && zombies[i].state != DEATH && zombies[i].state != DEAD) {
                // mgba_printf("zombie number %d", i);
                // play player damaged sound
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
        for(int i=0; i<TOTALREAVERS; i++) {
            if(reavers[i].active && reavers[i].state != DEATH && reavers[i].state != DEAD) {
                // play player damaged sound
                if(zombies[i].looking == L) {
                    if(player.invincibleTick == 0 && collision(reavers[i].x, reavers[i].y, reavers[i].width, reavers[i].height, player.x, SHIFTDOWN(player.y), player.width, player.height)) {
                        player.health--;
                        player.invincibleTick = 30;
                        shadowOAM[hearts[player.health].oamIndex].attr0 |= ATTR0_HIDE;
                        hearts[player.health].active = 0;
                    }
                } else {
                    if(player.invincibleTick == 0 && collision(reavers[i].x + 8, reavers[i].y, reavers[i].width, reavers[i].height, player.x, SHIFTDOWN(player.y), player.width, player.height)) {
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

void updateLevel() {
    updatePlayer();
    updateEnemies();

    hOff = player.x - (SCREENWIDTH - player.width) / 2;
    vOff = SHIFTDOWN(player.y) - (SCREENHEIGHT - player.height) / 2;
    bghOff = ((player.x / 2) - (SCREENWIDTH - player.width) / 2) + 58;
    bgvOff = ((SHIFTDOWN(player.y) / 2) - (SCREENHEIGHT - player.height) / 2) + 39;

    if(currentLevel == 1) {
        updateLevel1Offsets();
    } else if(currentLevel == 2) {
        updateLevel2Offsets();
    } else if(currentLevel==3) {
        updateLevel3Offsets();
    }
}

void drawLevel() {
    drawPlayer();
    drawEnemies();
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
        currentLevel++;
        initNext();
        // goToWin();
    }

    REG_BG2HOFF = hOff;
    REG_BG2VOFF = vOff;
    REG_BG3HOFF = bghOff;
    // mgba_printf("bgvOff: %d", bgvOff);
    // mgba_printf("vOff: %d", vOff);
    REG_BG3VOFF = bgvOff;

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
    if (player.state == JUMP) {
        shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(playerPal) | ATTR2_TILEID(2, 0);
    } 
    if (player.state == FALL) {
        shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(playerPal) | ATTR2_TILEID(4, 0);
    } else {
        shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(playerPal) | ATTR2_TILEID(2 * player.currentFrame, 2 * player.state);
    }
    if(slash.isAnimating) { 
        shadowOAM[player.oamIndex].attr2 = ATTR2_PALROW(playerPal) | ATTR2_TILEID(6, 0);
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

drawEnemies() {
    for(int i=0; i<TOTALZOMBIES; i++) {
        if(zombies[i].active && zombies[i].state != DEAD) {
            // mgba_printf("enemy: %d, current: %d, state: %d", i+1, zombies[i].currentFrame, zombies[i].state);
            // mgba_printf("enemy position: %d, %d", zombies[i].x, zombies[i].y);
            int y_diff = zombies[i].y - vOff;
            if(currentLevel == 2) {
                shadowOAM[zombies[i].oamIndex].attr0 = ATTR0_Y(zombies[i].y - vOff) | ATTR0_SQUARE | (y_diff > 160 || y_diff < 32 ? ATTR0_HIDE : ATTR0_REGULAR);
            } else {
                shadowOAM[zombies[i].oamIndex].attr0 = ATTR0_Y(zombies[i].y - vOff) | ATTR0_SQUARE;
            }
            
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
    for(int i=0; i<TOTALREAVERS; i++) {
        if(reavers[i].active && reavers[i].state != DEAD) {
            // mgba_printf("active reaver x: %d", reavers[i].x);
            int y_diff = reavers[i].y - vOff;
            shadowOAM[reavers[i].oamIndex].attr0 = ATTR0_Y(reavers[i].y - vOff) | ATTR0_WIDE | (y_diff > 160 || y_diff < 32 ? ATTR0_HIDE : ATTR0_REGULAR);
            shadowOAM[reavers[i].oamIndex].attr1 = ATTR1_X(reavers[i].x - hOff) | ATTR1_LARGE;
            if(reavers[i].state == SPAWN || reavers[i].state == ROAM || reavers[i].state == CHASE) {
                if(reavers[i].currentFrame < 4) {
                    if(reavers[i].beenhit) {
                        shadowOAM[reavers[i].oamIndex].attr2 = ATTR2_PALROW(2) | ATTR2_TILEID(reavers[i].currentFrame * 8, 12);
                    } else {
                        shadowOAM[reavers[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(reavers[i].currentFrame * 8, 12);
                    }
                } else {
                    if(reavers[i].beenhit) {
                        shadowOAM[reavers[i].oamIndex].attr2 = ATTR2_PALROW(2) | ATTR2_TILEID((reavers[i].currentFrame - 4) * 8, 16);
                    } else {
                        shadowOAM[reavers[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID((reavers[i].currentFrame - 4) * 8, 16);
                    }
                }
            }
            if(reavers[i].state == DEATH) {
                shadowOAM[reavers[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(24, 16);
            }
            if(!reavers[i].looking) {
                shadowOAM[reavers[i].oamIndex].attr1 |= ATTR1_HFLIP; 
            } 
        }
    }
}

void drawUI() {
    for(int i=0; i<player.health; i++) {
        // mgba_printf("heart info: %d", hearts[i].x);
        if(hearts[i].active) {
            shadowOAM[hearts[i].oamIndex].attr0 = ATTR0_Y(hearts[i].y) | ATTR0_SQUARE;
            shadowOAM[hearts[i].oamIndex].attr1 = ATTR1_X(hearts[i].x) | ATTR1_SMALL;
            shadowOAM[hearts[i].oamIndex].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(16, 0);
        }
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

inline unsigned char colorAt(int x, int y){
    if(currentLevel == 1) {
        return ((unsigned char *) level1collisionsBitmap) [OFFSET(x, y, MAP1WIDTH)];
    } else if(currentLevel == 2) {
        return ((unsigned char *) level2collisionsBitmap) [OFFSET(x, y, MAP2WIDTH)];
    } else if(currentLevel == 3) {
        return ((unsigned char *) level3collisionsBitmap) [OFFSET(x, y, MAP3WIDTH)];
    }
}

void wipeZombies() {
    for(int i=0; i<TOTALZOMBIES; i++) {
        zombies[i].health = 0;
        zombies[i].numFrames = 0;
        zombies[i].currentFrame = 0;
        zombies[i].state = 0;
        zombies[i].looking = 0;
        zombies[i].timeUntilNextFrame = 0;
        zombies[i].width = 0;
        zombies[i].height = 0;
        zombies[i].idleTick = 0;
        zombies[i].beenhit = 0;
        zombies[i].active = 0;
    }
}