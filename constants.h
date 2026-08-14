#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdbool.h>

#define ROWS 23
#define COLS 21
#define CELL_SIZE 22.0f

#define NUM_GHOSTS 4
#define FRIGHT_DURATION 400

#define WALL      'W'
#define DOT       '.'
#define ENERGIZER 'O'
#define PORTAL    'E'
#define EMPTY     ' '
#define GHOST_CHAR 'G'

#define MAX_SCORES 10
#define PAC_SPEED   7
#define GHOST_SPEED 10
#define TICK_DURATION 0.025

typedef struct {
    int x, y;
    int dx, dy;
    int nextDx, nextDy;
} Pacman;

typedef struct {
    int x, y;
    int startX, startY;
    float r, g, b;
    char under;
} Ghost;

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_HELP,
    STATE_ABOUT
} GameState;

// Глобальные переменные, определённые в game.c
extern char grid[ROWS][COLS];
extern Pacman pacman;
extern Ghost ghosts[NUM_GHOSTS];
extern int score;
extern int lives;
extern int dotsLeft;
extern bool frightened;
extern int frightTimer;
extern bool gameOver;
extern bool win;
extern int globalTick;
extern double lastUpdateTime;

extern int highScores[MAX_SCORES];
extern int numScores;

extern GameState currentState;
extern int selectedMenuItem;
extern const char* menuItems[];
extern int menuSize;

#endif