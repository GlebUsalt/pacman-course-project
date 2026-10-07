#include "game.h"
#include "highscores.h"
#include <stdlib.h>

#define PATH_INF 10000


static const char* MAP_TEMPLATE[ROWS] = {
    "WWWWWWWWWWWWWWWWWWWWW",
    "WO.......WWW.......OW",
    "W........WWW........W",
    "W..WWW...WWW...WWW..W",
    "W..W W...WWW...W W..W",
    "W..WWW...WWW...WWW..W",
    "W......WWWWWWW......W",
    "W.W....WWWWWWW....W.W",
    "W...................W",
    "WWWW.W.WWWWWWW.W.WWWW",
    "WWWW.W.W     W.W.WWWW",
    "E....W.W     W.W....E",
    "WWWW.W.W     W.W.WWWW",
    "WWWW.W.W     W.W.WWWW",
    "W......WWW WWW......W",
    "W..WWW.........WWW..W",
    "W....W...WWW...W....W",
    "W....W.W..W..W.W....W",
    "W.W..W.W..W..W.W..W.W",
    "W.W....W.. ..W....W.W",
    "W.WWWWWW.....WWWWWW.W",
    "WO.................OW",
    "WWWWWWWWWWWWWWWWWWWWW"
};

static const int GHOST_START_ROW[NUM_GHOSTS] = { 10, 10, 11, 11 };
static const int GHOST_START_COL[NUM_GHOSTS] = { 10, 11, 11, 10 };

static const int SCATTER_ROW[NUM_GHOSTS] = { 1, 1, 21, 21 };
static const int SCATTER_COL[NUM_GHOSTS] = { 19, 1, 19, 1 };

static const int DIR_R[4] = { -1, 1, 0, 0 };
static const int DIR_C[4] = {  0, 0, -1, 1 };

static const float GHOST_COLORS[NUM_GHOSTS][3] = {
    {1.0f, 0.0f, 0.0f},
    {1.0f, 0.5f, 0.8f},
    {0.3f, 1.0f, 1.0f},
    {0.9f, 0.6f, 0.2f}
};

static const int PACMAN_START_ROW = 19;
static const int PACMAN_START_COL = 10;


static int findGhostAt(const Game* game, int row, int col) {
    for (int g = 0; g < NUM_GHOSTS; g++) {
        if (game->ghosts[g].row == row && game->ghosts[g].col == col)
            return g;
    }
    return -1;
}

static void eatGhost(Game* game, int ghostIdx) {
    game->score += 200;
    Ghost* ghost = &game->ghosts[ghostIdx];
    ghost->row = ghost->startRow;
    ghost->col = ghost->startCol;
    ghost->prevDRow = 0;
    ghost->prevDCol = 0;
}

static void killPacman(Game* game) {
    game->lives--;
    if (game->lives > 0) {
        resetAfterDeath(game);
    } else {
        game->gameOver = true;
        addHighScore(game->score);
    }
}

static void consumeCell(Game* game, int row, int col, char cell) {
    if (cell == DOT) {
        game->score += 10;
        game->dotsLeft--;
    } else if (cell == ENERGIZER) {
        game->score += 50;
        game->dotsLeft--;
        game->frightened = true;
        game->frightTimer = FRIGHT_DURATION;
    }
    game->grid[row][col] = EMPTY;
}

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static bool stepCell(const Game* game, int row, int col, int d,
                     int* outRow, int* outCol) {
    int r = row + DIR_R[d];
    int c = col + DIR_C[d];
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    char cell = game->grid[r][c];
    if (cell == WALL) return false;
    if (cell == PORTAL) {
        if (c == 0)             c = COLS - 2;
        else if (c == COLS - 1) c = 1;
        if (game->grid[r][c] == WALL) return false;
    }
    *outRow = r;
    *outCol = c;
    return true;
}

static void bfsFrom(const Game* game, int startRow, int startCol,
                    int dist[ROWS][COLS]) {
    int qr[ROWS * COLS], qc[ROWS * COLS];
    int head = 0, tail = 0;

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            dist[r][c] = PATH_INF;

    dist[startRow][startCol] = 0;
    qr[tail] = startRow;
    qc[tail] = startCol;
    tail++;

    while (head < tail) {
        int r = qr[head], c = qc[head];
        head++;
        for (int d = 0; d < 4; d++) {
            int nr, nc;
            if (!stepCell(game, r, c, d, &nr, &nc)) continue;
            if (dist[nr][nc] != PATH_INF) continue;
            dist[nr][nc] = dist[r][c] + 1;
            qr[tail] = nr;
            qc[tail] = nc;
            tail++;
        }
    }
}

static void snapToOpen(const Game* game, int* row, int* col) {
    int r0 = *row, c0 = *col;
    for (int rad = 0; rad < ROWS + COLS; rad++) {
        for (int dr = -rad; dr <= rad; dr++) {
            int rest = rad - abs(dr);
            for (int s = 0; s < 2; s++) {
                if (s == 1 && rest == 0) continue;
                int dc = s == 0 ? rest : -rest;
                int r = r0 + dr, c = c0 + dc;
                if (r < 0 || r >= ROWS || c < 0 || c >= COLS) continue;
                char cell = game->grid[r][c];
                if (cell == WALL || cell == PORTAL || cell == '\0') continue;
                *row = r;
                *col = c;
                return;
            }
        }
    }
}

static int nearestGhostTime(int ghostDist[NUM_GHOSTS][ROWS][COLS],
                            int row, int col) {
    int best = PATH_INF * GHOST_SPEED;
    for (int g = 0; g < NUM_GHOSTS; g++) {
        int t = ghostDist[g][row][col] * GHOST_SPEED;
        if (t < best) best = t;
    }
    return best;
}

static int ringRadius(int freePercent) {
    if (freePercent >= 60) return 8;
    if (freePercent >= 35) return 6;
    return 4;
}

static int crowdPenalty(const Game* game, int g, int row, int col) {
    int penalty = 0;
    for (int o = 0; o < NUM_GHOSTS; o++) {
        if (o == g) continue;
        int d = abs(row - game->ghosts[o].row) + abs(col - game->ghosts[o].col);
        if (d < CROWD_RADIUS) penalty += (CROWD_RADIUS - d) * CROWD_WEIGHT;
    }
    return penalty;
}

static void updateCoordinator(Game* game) {
    const Pacman* p = &game->pacman;

    game->chaseMode =
        (game->globalTick % (SCATTER_TICKS + CHASE_TICKS)) >= SCATTER_TICKS;

    if (!game->chaseMode) {
        for (int g = 0; g < NUM_GHOSTS; g++) {
            game->targetRow[g] = SCATTER_ROW[g];
            game->targetCol[g] = SCATTER_COL[g];
        }
        return;
    }

    int pacDist[ROWS][COLS];
    int ghostDist[NUM_GHOSTS][ROWS][COLS];
    bfsFrom(game, p->row, p->col, pacDist);
    for (int g = 0; g < NUM_GHOSTS; g++)
        bfsFrom(game, game->ghosts[g].row, game->ghosts[g].col, ghostDist[g]);

    int total = 0, freeCount = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (pacDist[r][c] >= PATH_INF) continue;
            total++;
            if (pacDist[r][c] * PAC_SPEED < nearestGhostTime(ghostDist, r, c))
                freeCount++;
        }
    }
    int freePercent = total > 0 ? freeCount * 100 / total : 0;

    int ringR[ROWS * COLS], ringC[ROWS * COLS];
    int ringCount = 0;
    for (int radius = ringRadius(freePercent);
         radius >= 2 && ringCount == 0; radius--) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (pacDist[r][c] != radius) continue;
                if (pacDist[r][c] * PAC_SPEED >= nearestGhostTime(ghostDist, r, c))
                    continue;
                ringR[ringCount] = r;
                ringC[ringCount] = c;
                ringCount++;
            }
        }
    }

    for (int g = 0; g < NUM_GHOSTS; g++) {
        game->targetRow[g] = p->row;
        game->targetCol[g] = p->col;
    }

    if (ringCount == 0 || freePercent < SQUEEZE_PERCENT) return;

    bool assigned[NUM_GHOSTS];
    bool used[ROWS * COLS];
    for (int g = 0; g < NUM_GHOSTS; g++) assigned[g] = (g == 0);
    for (int i = 0; i < ringCount; i++) used[i] = false;

    for (int round = 1; round < NUM_GHOSTS; round++) {
        int bestG = -1, bestI = -1, bestScore = 1 << 30;
        for (int g = 1; g < NUM_GHOSTS; g++) {
            if (assigned[g]) continue;
            for (int i = 0; i < ringCount; i++) {
                if (used[i]) continue;
                int score = ghostDist[g][ringR[i]][ringC[i]] * PATH_WEIGHT;
                for (int o = 1; o < NUM_GHOSTS; o++) {
                    if (!assigned[o]) continue;
                    int d = abs(ringR[i] - game->targetRow[o]) +
                            abs(ringC[i] - game->targetCol[o]);
                    if (d < SPREAD_RADIUS)
                        score += (SPREAD_RADIUS - d) * SPREAD_WEIGHT;
                }
                if (score < bestScore) {
                    bestScore = score;
                    bestG = g;
                    bestI = i;
                }
            }
        }
        if (bestG == -1) break;
        game->targetRow[bestG] = ringR[bestI];
        game->targetCol[bestG] = ringC[bestI];
        assigned[bestG] = true;
        used[bestI] = true;
    }

    for (int g = 1; g < NUM_GHOSTS; g++) {
        if (assigned[g]) continue;
        game->targetRow[g] = clampi(p->row + 2 * g * p->dRow, 0, ROWS - 1);
        game->targetCol[g] = clampi(p->col + 2 * g * p->dCol, 0, COLS - 1);
        snapToOpen(game, &game->targetRow[g], &game->targetCol[g]);
    }
}


void initGame(Game* game) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            game->grid[r][c] = MAP_TEMPLATE[r][c];

    game->pacman.row = PACMAN_START_ROW;
    game->pacman.col = PACMAN_START_COL;
    game->pacman.dRow = 0;
    game->pacman.dCol = -1;
    game->pacman.nextDRow = 0;
    game->pacman.nextDCol = -1;

    game->dotsLeft = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (game->grid[r][c] == DOT || game->grid[r][c] == ENERGIZER)
                game->dotsLeft++;

    for (int g = 0; g < NUM_GHOSTS; g++) {
        Ghost* ghost = &game->ghosts[g];
        ghost->row = GHOST_START_ROW[g];
        ghost->col = GHOST_START_COL[g];
        ghost->startRow = GHOST_START_ROW[g];
        ghost->startCol = GHOST_START_COL[g];
        ghost->r = GHOST_COLORS[g][0];
        ghost->g = GHOST_COLORS[g][1];
        ghost->b = GHOST_COLORS[g][2];
        ghost->prevDRow = 0;
        ghost->prevDCol = 0;
        game->targetRow[g] = SCATTER_ROW[g];
        game->targetCol[g] = SCATTER_COL[g];
    }

    game->score = 0;
    game->lives = 3;
    game->frightened = false;
    game->frightTimer = 0;
    game->gameOver = false;
    game->win = false;
    game->globalTick = 0;
    game->chaseMode = false;
}


void movePacman(Game* game) {
    if (game->gameOver || game->win) return;

    Pacman* p = &game->pacman;

    int tryRow = p->row + p->nextDRow;
    int tryCol = p->col + p->nextDCol;
    if (tryRow >= 0 && tryRow < ROWS && tryCol >= 0 && tryCol < COLS &&
        game->grid[tryRow][tryCol] != WALL) {
        p->dRow = p->nextDRow;
        p->dCol = p->nextDCol;
    }

    int newRow = p->row + p->dRow;
    int newCol = p->col + p->dCol;
    if (newRow < 0 || newRow >= ROWS || newCol < 0 || newCol >= COLS) return;

    char target = game->grid[newRow][newCol];
    if (target == WALL) return;

    if (target == PORTAL) {
        int destCol = -1;
        if (newCol == 0)                destCol = COLS - 2;
        else if (newCol == COLS - 1)    destCol = 1;
        if (destCol == -1 || game->grid[newRow][destCol] == WALL) return;

        int ghostIdx = findGhostAt(game, newRow, destCol);
        if (ghostIdx != -1) {
            if (game->frightened) {
                eatGhost(game, ghostIdx);
            } else {
                killPacman(game);
                return;
            }
        }
        p->row = newRow;
        p->col = destCol;
        consumeCell(game, p->row, p->col, game->grid[p->row][p->col]);
        return;
    }

    int ghostIdx = findGhostAt(game, newRow, newCol);
    if (ghostIdx != -1) {
        if (game->frightened) {
            eatGhost(game, ghostIdx);
        } else {
            killPacman(game);
            return;
        }
    }

    p->row = newRow;
    p->col = newCol;
    consumeCell(game, p->row, p->col, target);
}


void moveGhosts(Game* game) {
    if (game->gameOver || game->win) return;

    if (!game->frightened) updateCoordinator(game);

    for (int g = 0; g < NUM_GHOSTS; g++) {
        Ghost* ghost = &game->ghosts[g];

        int pathDist[ROWS][COLS];
        if (game->frightened)
            bfsFrom(game, game->pacman.row, game->pacman.col, pathDist);
        else
            bfsFrom(game, game->targetRow[g], game->targetCol[g], pathDist);

        int validDirs[4], validRow[4], validCol[4];
        int validCount = 0;
        for (int d = 0; d < 4; d++) {
            int nr, nc;
            if (!stepCell(game, ghost->row, ghost->col, d, &nr, &nc)) continue;
            int other = findGhostAt(game, nr, nc);
            if (other != -1 && other != g) continue;
            validDirs[validCount] = d;
            validRow[validCount] = nr;
            validCol[validCount] = nc;
            validCount++;
        }
        if (validCount == 0) continue;

        int bestIdx = -1;
        int bestScore = 1 << 30;

        for (int i = 0; i < validCount; i++) {
            int d = validDirs[i];
            if (validCount > 1 &&
                DIR_R[d] == -ghost->prevDRow &&
                DIR_C[d] == -ghost->prevDCol)
                continue;

            int nr = validRow[i], nc = validCol[i];
            int score = pathDist[nr][nc] * PATH_WEIGHT;
            if (game->frightened) score = -score;
            score += crowdPenalty(game, g, nr, nc);

            if (score < bestScore) {
                bestScore = score;
                bestIdx = i;
            }
        }
        if (bestIdx == -1) bestIdx = 0;

        ghost->row = validRow[bestIdx];
        ghost->col = validCol[bestIdx];
        ghost->prevDRow = DIR_R[validDirs[bestIdx]];
        ghost->prevDCol = DIR_C[validDirs[bestIdx]];

        if (ghost->row == game->pacman.row &&
            ghost->col == game->pacman.col) {
            if (game->frightened) {
                eatGhost(game, g);
            } else {
                killPacman(game);
                return;
            }
        }
    }
}


void resetAfterDeath(Game* game) {
    game->pacman.row = PACMAN_START_ROW;
    game->pacman.col = PACMAN_START_COL;
    game->pacman.dRow = 0;
    game->pacman.dCol = -1;
    game->pacman.nextDRow = 0;
    game->pacman.nextDCol = -1;

    for (int g = 0; g < NUM_GHOSTS; g++) {
        Ghost* ghost = &game->ghosts[g];
        ghost->row = ghost->startRow;
        ghost->col = ghost->startCol;
        ghost->prevDRow = 0;
        ghost->prevDCol = 0;
    }

    game->frightened = false;
    game->frightTimer = 0;
}
