#include "sprites.h"
#include "gba.h"
#include "gameTiles.h"
#include "print.h"
#include "level2.h"
#include "game.h"
#include "mode0.h"
#include "spritesheet.h"
#include "background1.h"
#include "zombiedeath.h"
#include "attack.h"
#include "hurt.h"
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