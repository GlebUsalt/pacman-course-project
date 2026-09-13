#include "game.h"
#include "highscores.h"
#include <stdlib.h>

/*
 * Карта уровня. Все стены, точки, энергайзеры, порталы и пустые клетки
 * задаются здесь как данные. Символы:
 *   W - стена        . - точка       O - энергайзер
 *   E - портал       ' ' - пусто
 * Клетки старта пакмана и призраков должны быть пустыми (' ').
 */
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
    "WWWW.W.WWWWWWW.W.WWW",
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

static const float GHOST_COLORS[NUM_GHOSTS][3] = {
    {1.0f, 0.0f, 0.0f},
    {1.0f, 0.5f, 0.8f},
    {0.3f, 1.0f, 1.0f},
    {0.9f, 0.6f, 0.2f}
};

static const int PACMAN_START_ROW = 19;
static const int PACMAN_START_COL = 10;

/* ---------- Хелперы ---------- */

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

/* ---------- Инициализация ---------- */

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
    }

    game->score = 0;
    game->lives = 3;
    game->frightened = false;
    game->frightTimer = 0;
    game->gameOver = false;
    game->win = false;
    game->globalTick = 0;
}

/* ---------- Движение пакмана ---------- */

void movePacman(Game* game) {
    if (game->gameOver || game->win) return;

    Pacman* p = &game->pacman;

    /* Пытаемся применить буферизованное направление */
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

    /* Портал */
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

    /* Столкновение с призраком */
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

/* ---------- Движение призраков ---------- */

void moveGhosts(Game* game) {
    if (game->gameOver || game->win) return;

    static const int dRows[NUM_GHOSTS] = { -1, 1, 0, 0 };
    static const int dCols[NUM_GHOSTS] = {  0, 0, -1, 1 };

    for (int g = 0; g < NUM_GHOSTS; g++) {
        Ghost* ghost = &game->ghosts[g];

        /* Собираем допустимые направления */
        int validDirs[4];
        int validCount = 0;
        for (int d = 0; d < 4; d++) {
            int nr = ghost->row + dRows[d];
            int nc = ghost->col + dCols[d];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (game->grid[nr][nc] == WALL) continue;
            int other = findGhostAt(game, nr, nc);
            if (other != -1 && other != g) continue;
            validDirs[validCount++] = d;
        }
        if (validCount == 0) continue;

        /* Выбор направления: жадно к пакману, либо от него (frightened) */
        int bestDir = -1;
        int bestDist = game->frightened ? -1 : 10000;

        for (int i = 0; i < validCount; i++) {
            int d = validDirs[i];
            /* Запрещаем разворот, если есть альтернатива */
            if (validCount > 1 &&
                dRows[d] == -ghost->prevDRow &&
                dCols[d] == -ghost->prevDCol)
                continue;

            int nr = ghost->row + dRows[d];
            int nc = ghost->col + dCols[d];
            int dist = abs(nr - game->pacman.row) + abs(nc - game->pacman.col);

            if (game->frightened) {
                if (dist > bestDist) { bestDist = dist; bestDir = d; }
            } else {
                if (dist < bestDist) { bestDist = dist; bestDir = d; }
            }
        }
        if (bestDir == -1) bestDir = validDirs[0];

        int newRow = ghost->row + dRows[bestDir];
        int newCol = ghost->col + dCols[bestDir];

        /* Портал */
        if (game->grid[newRow][newCol] == PORTAL) {
            if (newCol == 0)                newCol = COLS - 2;
            else if (newCol == COLS - 1)    newCol = 1;
        }

        ghost->row = newRow;
        ghost->col = newCol;
        ghost->prevDRow = dRows[bestDir];
        ghost->prevDCol = dCols[bestDir];

        /* Столкновение с пакманом */
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

/* ---------- Возрождение после смерти ---------- */

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
