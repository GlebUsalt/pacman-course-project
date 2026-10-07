#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define ROWS 23
#define COLS 21
#define CELL_SIZE 22.0f

#define NUM_GHOSTS 4
#define FRIGHT_DURATION 400

#define PAC_SPEED   7
#define GHOST_SPEED 10
#define TICK_DURATION 0.025

#define MAX_TICKS_PER_FRAME 5

#define SCATTER_TICKS 280
#define CHASE_TICKS 800

#define PATH_WEIGHT 4
#define CROWD_RADIUS 3
#define CROWD_WEIGHT 2
#define SPREAD_RADIUS 6
#define SPREAD_WEIGHT 3
#define SQUEEZE_PERCENT 12

#define WALL       'W'
#define DOT        '.'
#define ENERGIZER  'O'
#define PORTAL     'E'
#define EMPTY      ' '

typedef struct {
    int row, col;
    int dRow, dCol;
    int nextDRow, nextDCol;
} Pacman;

typedef struct {
    int row, col;
    int startRow, startCol;
    float r, g, b;
    int prevDRow, prevDCol;
} Ghost;

typedef struct {
    char grid[ROWS][COLS];
    Pacman pacman;
    Ghost ghosts[NUM_GHOSTS];
    int score;
    int lives;
    int dotsLeft;
    bool frightened;
    int frightTimer;
    bool gameOver;
    bool win;
    int globalTick;
    bool chaseMode;
    int targetRow[NUM_GHOSTS];
    int targetCol[NUM_GHOSTS];
} Game;

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_HELP,
    STATE_ABOUT
} GameState;

void initGame(Game* game);
void movePacman(Game* game);
void moveGhosts(Game* game);
void resetAfterDeath(Game* game);

#endif
