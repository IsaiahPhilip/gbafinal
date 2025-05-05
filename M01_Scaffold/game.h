#define GAME_H

#include "sprites.h"

#define GRAVITY 80 // How fast things fall
#define TERMINALVELOCITY 1300 // Fastest possible fall speed
#define DAMAGEVELOCITY 1000
#define LASTLEVEL 3
#define TOTALZOMBIES 9
#define TOTALREAVERS 3

// For 8.8 fixed point encoding
#define SHIFTUP(n) ((n) << 8)
#define SHIFTDOWN(n) ((n) >> 8)

// Returns value if min <= value <= max, returns min if value < min, returns max if value > max
#define CLAMP(value, min, max) (((min) * ((value) < (min))) + ((max) * ((value) > (max))) + ((value) * ((value) >= (min) && (value) <= (max))))

// Returns value if value >= floor, returns floor if value < floor
#define FLOOR(value, floor) ((floor) * ((value) < (floor)) + (value) * ((value) >= (floor)))

// Returns value if value <= ceiling, returns ceiling if value > ceiling
#define CEILING(value, ceiling) ((ceiling) * ((value) > (ceiling)) + (value) * ((value) <= (ceiling)))

extern int score;
extern int finalScore;
extern int currentLevel;
// set equal to 0 initially
// if attackCooldown == 0 when button pressed to attack
// if attackCooldown > 0 -> attackCooldown--;
extern int attackCooldown;
// variable for oscillating beavior
extern int gameTick; 

// these general functions will call the function specific to each level
void initGame(); // initial values for each playthrough
void initLevel(); // set initial game logic for level, pretty much everything except map related stuff
void initMap(); // only initialize map, used to decouple w/ game logic for pause handling
void initNext();
void updateLevel(); // updates the game logic according to the current level
void updatePlayer();
void playerAttack();
void slashEnemy();
void updateEnemies();
void updateZombies();
void drawLevel();
void drawUI();
extern OBJ_ATTR shadowOAM[128];

extern PLAYER player;
extern SPRITE slash;
extern OBJECT hearts[3];
extern ENEMY zombies[TOTALZOMBIES];
extern ENEMY reavers[TOTALREAVERS];

// extern typedef enum {DOWN, RIGHT, UP, LEFT} DIRECTION;

extern int vOff;
extern int hOff;
extern int bgvOff;
extern int bghOff;