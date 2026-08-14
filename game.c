#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "constants.h"
#include "game.h"
#include "highscores.h"

char grid[ROWS][COLS];
Pacman pacman;
Ghost ghosts[NUM_GHOSTS];
int score = 0;
int lives = 3;
int dotsLeft = 0;
bool frightened = false;
int frightTimer = 0;
bool gameOver = false;
bool win = false;
int globalTick = 0;
double lastUpdateTime = 0.0;

static int ghostStartX[NUM_GHOSTS] = { 10, 10, 9, 11 };
static int ghostStartY[NUM_GHOSTS] = { 10, 11, 10, 10 };
static float ghostColors[NUM_GHOSTS][3] = {
    {1.0f, 0.0f, 0.0f},
    {1.0f, 0.5f, 0.8f},
    {0.3f, 1.0f, 1.0f},
    {0.9f, 0.6f, 0.2f}
};

void initGame() {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++) grid[i][j] = DOT;
    for (int i = 0; i < ROWS; i++) { grid[i][0] = WALL; grid[i][COLS - 1] = WALL; }
    for (int j = 0; j < COLS; j++) { grid[0][j] = WALL; grid[ROWS - 1][j] = WALL; }

    int top = 9, bottom = 14, left = 7, right = 13;
    for (int i = top; i <= bottom; i++) { grid[i][left] = WALL; grid[i][right] = WALL; }
    for (int j = left; j <= right; j++) grid[top][j] = WALL;
    int entrance = (left + right) / 2;
    for (int j = left; j <= right; j++) {
        if (j == entrance) grid[bottom][j] = EMPTY;
        else grid[bottom][j] = WALL;
    }
    for (int i = top + 1; i < bottom; i++)
        for (int j = left + 1; j < right; j++)
            if (grid[i][j] == DOT) grid[i][j] = EMPTY;

    for (int j = 3; j <= 5; j++) {
        grid[3][j] = WALL;
        if (j == 4) grid[4][j] = EMPTY; else grid[4][j] = WALL;
        grid[5][j] = WALL;
    }
    for (int j = 15; j <= 17; j++) {
        grid[3][j] = WALL;
        if (j == 16) grid[4][j] = EMPTY; else grid[4][j] = WALL;
        grid[5][j] = WALL;
    }
    for (int j = 1; j <= 3; j++) { grid[9][j] = WALL; grid[10][j] = WALL; grid[12][j] = WALL; grid[13][j] = WALL; }
    for (int j = 7; j <= 13; j++) {
        grid[7][j] = WALL;
        if (j == 7 || j == 13) grid[6][j] = WALL;
    }
    for (int j = 8; j <= 12; j++) grid[6][j] = WALL;
    for (int j = 17; j <= 19; j++) { grid[9][j] = WALL; grid[10][j] = WALL; grid[12][j] = WALL; grid[13][j] = WALL; }
    for (int j = 3; j <= 5; j++) grid[15][j] = WALL;
    for (int i = 16; i <= 18; i++) grid[i][5] = WALL;
    for (int i = 18; i <= 20; i++) grid[i][2] = WALL;
    for (int j = 2; j <= 7; j++) grid[20][j] = WALL;
    for (int i = 17; i <= 20; i++) grid[i][7] = WALL;
    for (int j = 15; j <= 17; j++) grid[15][j] = WALL;
    for (int i = 16; i <= 18; i++) grid[i][15] = WALL;
    for (int i = 18; i <= 20; i++) grid[i][18] = WALL;
    for (int j = 13; j <= 18; j++) grid[20][j] = WALL;
    for (int i = 17; i <= 20; i++) grid[i][13] = WALL;
    for (int j = 9; j <= 11; j++) grid[16][j] = WALL;
    for (int i = 16; i <= 18; i++) grid[i][10] = WALL;
    grid[7][2] = WALL; grid[7][18] = WALL;
    for (int i = 9; i <= 13; i++) {
        grid[i][5] = WALL;
        grid[i][15] = WALL;
    }
    for (int i = 1; i <= 5; i++) {
        grid[i][9] = WALL;
        grid[i][10] = WALL;
        grid[i][11] = WALL;
    }
    grid[1][1] = ENERGIZER;
    grid[1][COLS - 2] = ENERGIZER;
    grid[ROWS - 2][1] = ENERGIZER;
    grid[ROWS - 2][COLS - 2] = ENERGIZER;
    int midRow = ROWS / 2;
    grid[midRow][0] = PORTAL;
    grid[midRow][COLS - 1] = PORTAL;

    pacman.x = 19; pacman.y = 10;
    pacman.dx = 0; pacman.dy = -1;
    pacman.nextDx = 0; pacman.nextDy = -1;
    if (grid[pacman.x][pacman.y] == DOT || grid[pacman.x][pacman.y] == ENERGIZER) grid[pacman.x][pacman.y] = EMPTY;
    for (int g = 0; g < NUM_GHOSTS; g++) {
        int sx = ghostStartX[g], sy = ghostStartY[g];
        if (grid[sx][sy] == DOT || grid[sx][sy] == ENERGIZER) grid[sx][sy] = EMPTY;
    }

    dotsLeft = 0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (grid[i][j] == DOT || grid[i][j] == ENERGIZER) dotsLeft++;

    for (int g = 0; g < NUM_GHOSTS; g++) {
        ghosts[g].x = ghostStartX[g]; ghosts[g].y = ghostStartY[g];
        ghosts[g].startX = ghostStartX[g]; ghosts[g].startY = ghostStartY[g];
        ghosts[g].r = ghostColors[g][0]; ghosts[g].g = ghostColors[g][1]; ghosts[g].b = ghostColors[g][2];
        ghosts[g].under = grid[ghosts[g].x][ghosts[g].y];
        grid[ghosts[g].x][ghosts[g].y] = GHOST_CHAR;
    }
    grid[pacman.x][pacman.y] = EMPTY;

    score = 0; lives = 3; frightened = false; frightTimer = 0; gameOver = false; win = false;
}

void movePacman() {
    int nx = pacman.x + pacman.nextDx, ny = pacman.y + pacman.nextDy;
    if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS) {
        char c = grid[nx][ny];
        if (c != WALL && c != GHOST_CHAR) {
            pacman.dx = pacman.nextDx;
            pacman.dy = pacman.nextDy;
        }
    }
    int newX = pacman.x + pacman.dx, newY = pacman.y + pacman.dy;
    if (newX < 0 || newX >= ROWS || newY < 0 || newY >= COLS) {
        if (pacman.dx == 0 && newY < 0) newY = COLS - 1;
        else if (pacman.dx == 0 && newY >= COLS) newY = 0;
        else return;
    }
    char target = grid[newX][newY];
    if (target == WALL) return;
    if (target == GHOST_CHAR) {
        int idx = -1;
        for (int g = 0; g < NUM_GHOSTS; g++)
            if (ghosts[g].x == newX && ghosts[g].y == newY) { idx = g; break; }
        if (frightened && idx != -1) {
            score += 200;
            if (grid[ghosts[idx].x][ghosts[idx].y] == GHOST_CHAR)
                grid[ghosts[idx].x][ghosts[idx].y] = ghosts[idx].under;
            ghosts[idx].x = ghosts[idx].startX; ghosts[idx].y = ghosts[idx].startY;
            ghosts[idx].under = EMPTY;
            grid[ghosts[idx].x][ghosts[idx].y] = GHOST_CHAR;
        }
        else {
            lives--;
            if (lives > 0) resetAfterDeath();
            else gameOver = true;
            return;
        }
    }
    if (target == DOT) { score += 10; dotsLeft--; }
    else if (target == ENERGIZER) { score += 50; dotsLeft--; frightened = true; frightTimer = FRIGHT_DURATION; }
    grid[pacman.x][pacman.y] = EMPTY;
    pacman.x = newX; pacman.y = newY;
}

void moveGhosts() {
    const int ghostHouseTop = 9, ghostHouseLeft = 7, ghostHouseRight = 13;
    for (int g = 0; g < NUM_GHOSTS; g++) {
        int dx[] = { -1,1,0,0 }, dy[] = { 0,0,-1,1 };
        int bestDir = -1, bestDist = frightened ? -1 : 10000;
        for (int d = 0; d < 4; d++) {
            if (ghosts[g].x == ghostHouseTop && (ghosts[g].y >= ghostHouseLeft && ghosts[g].y <= ghostHouseRight) && dx[d] == -1)
                continue;
            int nx = ghosts[g].x + dx[d], ny = ghosts[g].y + dy[d];
            if (nx < 0 || nx >= ROWS || ny < 0 || ny >= COLS) continue;
            if (grid[nx][ny] == WALL || grid[nx][ny] == GHOST_CHAR) continue;
            if (!frightened) {
                if ((dx[d] == -1 && ghosts[g].x - nx < 0) || (dx[d] == 1 && ghosts[g].x - nx > 0) ||
                    (dy[d] == -1 && ghosts[g].y - ny < 0) || (dy[d] == 1 && ghosts[g].y - ny > 0))
                    continue;
            }
            int dist = abs(nx - pacman.x) + abs(ny - pacman.y);
            if (frightened) { if (dist > bestDist) { bestDist = dist; bestDir = d; } }
            else { if (dist < bestDist) { bestDist = dist; bestDir = d; } }
        }
        if (bestDir != -1) {
            int nx = ghosts[g].x + dx[bestDir], ny = ghosts[g].y + dy[bestDir];
            char oldUnder = ghosts[g].under;
            if (grid[ghosts[g].x][ghosts[g].y] == GHOST_CHAR) grid[ghosts[g].x][ghosts[g].y] = oldUnder;
            ghosts[g].x = nx; ghosts[g].y = ny;
            ghosts[g].under = grid[nx][ny];
            grid[nx][ny] = GHOST_CHAR;
            if (nx == pacman.x && ny == pacman.y) {
                if (frightened) {
                    score += 200;
                    grid[ghosts[g].x][ghosts[g].y] = ghosts[g].under;
                    ghosts[g].x = ghosts[g].startX; ghosts[g].y = ghosts[g].startY;
                    ghosts[g].under = EMPTY; grid[ghosts[g].x][ghosts[g].y] = GHOST_CHAR;
                }
                else {
                    lives--;
                    if (lives > 0) resetAfterDeath();
                    else gameOver = true;
                    return;
                }
            }
        }
    }
}

void resetAfterDeath() {
    pacman.x = 19; pacman.y = 10;
    pacman.dx = 0; pacman.dy = -1;
    pacman.nextDx = 0; pacman.nextDy = -1;
    for (int g = 0; g < NUM_GHOSTS; g++) {
        if (grid[ghosts[g].x][ghosts[g].y] == GHOST_CHAR) grid[ghosts[g].x][ghosts[g].y] = ghosts[g].under;
        ghosts[g].x = ghosts[g].startX; ghosts[g].y = ghosts[g].startY;
        ghosts[g].under = EMPTY;
        grid[ghosts[g].x][ghosts[g].y] = GHOST_CHAR;
    }
    frightened = false; frightTimer = 0;
}