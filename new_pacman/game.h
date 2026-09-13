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

/* ћаксимум игровых тиков за один кадр - защита от "spiral of death" */
#define MAX_TICKS_PER_FRAME 5

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
