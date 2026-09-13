#include "render.h"
#include "game.h"
#include "highscores.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

static const unsigned char fontData[][7] = {
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    {0x0E,0x11,0x02,0x04,0x08,0x10,0x1F},
    {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E},
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x10,0x13,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    {0x02,0x02,0x02,0x02,0x12,0x12,0x0C},
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    {0x11,0x1B,0x15,0x11,0x11,0x11,0x11},
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x0A,0x0A,0x04,0x04},
    {0x11,0x11,0x11,0x15,0x15,0x1B,0x11},
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}
};

static void drawChar(float x, float y, float size, char ch) {
    if (ch == ' ') return;

    if (ch == '.') {
        float s = size * 0.15f;
        glBegin(GL_QUADS);
        glVertex2f(x + size * 0.3f,           y - size * 0.75f);
        glVertex2f(x + size * 0.3f + s,       y - size * 0.75f);
        glVertex2f(x + size * 0.3f + s,       y - size * 0.75f + s);
        glVertex2f(x + size * 0.3f,           y - size * 0.75f + s);
        glEnd();
        return;
    }
    if (ch == ',') {
        glBegin(GL_TRIANGLES);
        glVertex2f(x + size * 0.2f, y - size * 0.8f);
        glVertex2f(x + size * 0.4f, y - size * 0.8f);
        glVertex2f(x + size * 0.3f, y - size * 0.6f);
        glEnd();
        return;
    }
    if (ch == ':') {
        float r = size * 0.1f;
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 2 * 3.14159f * k / 20;
            glVertex2f(x + size * 0.3f + cos(a) * r,
                       y - size * 0.3f + sin(a) * r);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 2 * 3.14159f * k / 20;
            glVertex2f(x + size * 0.3f + cos(a) * r,
                       y - size * 0.7f + sin(a) * r);
        }
        glEnd();
        return;
    }
    if (ch == '/') {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.1f, y - size * 0.9f);
        glVertex2f(x + size * 0.9f, y - size * 0.1f);
        glEnd();
        return;
    }
    if (ch == '%') {
        float r = size * 0.12f;
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 16; k++) {
            float a = 2 * 3.14159f * k / 16;
            glVertex2f(x + size * 0.22f + cos(a) * r,
                       y - size * 0.28f + sin(a) * r);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 16; k++) {
            float a = 2 * 3.14159f * k / 16;
            glVertex2f(x + size * 0.55f + cos(a) * r,
                       y - size * 0.72f + sin(a) * r);
        }
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.15f, y - size * 0.85f);
        glVertex2f(x + size * 0.62f, y - size * 0.15f);
        glEnd();
        return;
    }
    if (ch == '-') {
        glBegin(GL_QUADS);
        float w = size * 0.6f;
        float h = size * 0.1f;
        glVertex2f(x,          y - size * 0.5f);
        glVertex2f(x + w,      y - size * 0.5f);
        glVertex2f(x + w,      y - size * 0.5f - h);
        glVertex2f(x,          y - size * 0.5f - h);
        glEnd();
        return;
    }
    if (ch == '(' || ch == ')' || ch == '_') return;

    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'Z')) return;

    int idx;
    if (ch >= '0' && ch <= '9') idx = ch - '0';
    else                        idx = ch - 'A' + 10;
    if (idx < 0 || idx >= 36) return;

    float cellW = size * 0.6f / 5.0f;
    float cellH = size / 7.0f;
    for (int row = 0; row < 7; row++) {
        unsigned char mask = 0x10;
        for (int col = 0; col < 5; col++) {
            if (fontData[idx][row] & mask) {
                glBegin(GL_QUADS);
                glVertex2f(x + col * cellW,         y - row * cellH);
                glVertex2f(x + (col + 1) * cellW,   y - row * cellH);
                glVertex2f(x + (col + 1) * cellW,   y - (row + 1) * cellH);
                glVertex2f(x + col * cellW,         y - (row + 1) * cellH);
                glEnd();
            }
            mask >>= 1;
        }
    }
}

static void drawDigit(float x, float y, float h, int digit) {
    if (digit < 0 || digit > 9) return;
    float w = h * 0.6f, t = h * 0.1f;
    const bool segs[10][7] = {
        {1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},
        {0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},
        {1,1,1,1,1,1,1},{1,1,1,1,0,1,1}
    };
    const bool* s = segs[digit];
    glBegin(GL_QUADS);
    if (s[0]) { glVertex2f(x, y);           glVertex2f(x + w, y);
                glVertex2f(x + w, y - t);   glVertex2f(x, y - t); }
    if (s[1]) { glVertex2f(x + w, y);       glVertex2f(x + w, y - h / 2);
                glVertex2f(x + w - t, y - h / 2); glVertex2f(x + w - t, y); }
    if (s[2]) { glVertex2f(x + w, y - h / 2); glVertex2f(x + w, y - h);
                glVertex2f(x + w - t, y - h); glVertex2f(x + w - t, y - h / 2); }
    if (s[3]) { glVertex2f(x, y - h);       glVertex2f(x + w, y - h);
                glVertex2f(x + w, y - h + t); glVertex2f(x, y - h + t); }
    if (s[4]) { glVertex2f(x, y - h / 2);   glVertex2f(x, y - h);
                glVertex2f(x + t, y - h);   glVertex2f(x + t, y - h / 2); }
    if (s[5]) { glVertex2f(x, y);           glVertex2f(x, y - h / 2);
                glVertex2f(x + t, y - h / 2); glVertex2f(x + t, y); }
    if (s[6]) { float mid = y - h / 2 + t / 2;
                glVertex2f(x, mid);         glVertex2f(x + w, mid);
                glVertex2f(x + w, mid - t); glVertex2f(x, mid - t); }
    glEnd();
}

static void drawNumber(float x, float y, float h, int number) {
    if (number == 0) { drawDigit(x, y, h, 0); return; }
    char buf[12];
    sprintf(buf, "%d", number);
    float sp = h * 0.7f;
    for (int i = 0; buf[i]; i++)
        drawDigit(x + i * sp, y, h, buf[i] - '0');
}

void drawString(float x, float y, float size, const char* str) {
    float spacing = size * 0.7f;
    while (*str) {
        drawChar(x, y, size, *str);
        x += spacing;
        str++;
    }
}

void drawProgressBar(float x, float y, float w, float h,
                     float percent, const char* label) {
    glColor3f(0.2f, 0.2f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);         glVertex2f(x + w, y);
    glVertex2f(x + w, y - h); glVertex2f(x, y - h);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w * (percent / 100.0f), y);
    glVertex2f(x + w * (percent / 100.0f), y - h);
    glVertex2f(x, y - h);
    glEnd();

    glColor3f(1, 1, 1);
    drawString(x + w / 2 - 40, y - h / 2 - 5, 12, label);
    char buf[20];
    sprintf(buf, "%.0f%%", percent);
    drawString(x + w / 2 - 20, y - h / 2 - 25, 12, buf);
}

/* ---------- Отрисовка игровой сцены ---------- */

void drawScene(const Game* game) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    const float offsetX = 100.0f;
    const float offsetY = 50.0f;

    /* Статичные клетки */
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float cx = offsetX + c * CELL_SIZE;
            float cy = offsetY + (ROWS - 1 - r) * CELL_SIZE;
            char cell = game->grid[r][c];

            if (cell == WALL) {
                glColor3f(0, 0, 0.8f);
                glBegin(GL_QUADS);
                glVertex2f(cx, cy);
                glVertex2f(cx + CELL_SIZE, cy);
                glVertex2f(cx + CELL_SIZE, cy + CELL_SIZE);
                glVertex2f(cx, cy + CELL_SIZE);
                glEnd();
                glColor3f(1, 1, 1);
                glBegin(GL_LINE_LOOP);
                glVertex2f(cx, cy);
                glVertex2f(cx + CELL_SIZE, cy);
                glVertex2f(cx + CELL_SIZE, cy + CELL_SIZE);
                glVertex2f(cx, cy + CELL_SIZE);
                glEnd();
            } else if (cell == DOT) {
                glColor3f(1, 0.8f, 0.6f);
                float cx2 = cx + CELL_SIZE / 2, cy2 = cy + CELL_SIZE / 2;
                glBegin(GL_TRIANGLE_FAN);
                for (int k = 0; k <= 20; k++) {
                    float a = 2 * 3.14159f * k / 20;
                    glVertex2f(cx2 + cos(a) * 2.5f, cy2 + sin(a) * 2.5f);
                }
                glEnd();
            } else if (cell == ENERGIZER) {
                glColor3f(1, 0.8f, 0.6f);
                float cx2 = cx + CELL_SIZE / 2, cy2 = cy + CELL_SIZE / 2;
                glBegin(GL_TRIANGLE_FAN);
                for (int k = 0; k <= 20; k++) {
                    float a = 2 * 3.14159f * k / 20;
                    glVertex2f(cx2 + cos(a) * 6.0f, cy2 + sin(a) * 6.0f);
                }
                glEnd();
            } else if (cell == PORTAL) {
                float cx2 = cx + CELL_SIZE / 2, cy2 = cy + CELL_SIZE / 2;
                glColor3f(0.8f, 0.2f, 1.0f);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(cx2, cy2);
                for (int k = 0; k <= 20; k++) {
                    float a = 2 * 3.14159f * k / 20;
                    glVertex2f(cx2 + cos(a) * 8.0f, cy2 + sin(a) * 8.0f);
                }
                glEnd();
                glColor3f(0.0f, 0.0f, 0.0f);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(cx2, cy2);
                for (int k = 0; k <= 20; k++) {
                    float a = 2 * 3.14159f * k / 20;
                    glVertex2f(cx2 + cos(a) * 4.0f, cy2 + sin(a) * 4.0f);
                }
                glEnd();
            }
        }
    }

    /* Призраки */
    for (int g = 0; g < NUM_GHOSTS; g++) {
        const Ghost* ghost = &game->ghosts[g];
        float cx = offsetX + ghost->col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - ghost->row) * CELL_SIZE + CELL_SIZE / 2;

        if (game->frightened) glColor3f(0.3f, 0.3f, 1.0f);
        else                  glColor3f(ghost->r, ghost->g, ghost->b);

        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 3.14159f * k / 20;
            glVertex2f(cx + cos(a) * 9, cy + sin(a) * 9);
        }
        glEnd();
        glBegin(GL_QUADS);
        glVertex2f(cx - 9, cy);      glVertex2f(cx + 9, cy);
        glVertex2f(cx + 9, cy - 7);  glVertex2f(cx - 9, cy - 7);
        glEnd();

        glColor3f(1, 1, 1);
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 2 * 3.14159f * k / 20;
            glVertex2f(cx - 3.5f + cos(a) * 2.5f, cy + 2.5f + sin(a) * 2.5f);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 2 * 3.14159f * k / 20;
            glVertex2f(cx + 3.5f + cos(a) * 2.5f, cy + 2.5f + sin(a) * 2.5f);
        }
        glEnd();

        int dirRow = game->pacman.row - ghost->row;
        int dirCol = game->pacman.col - ghost->col;
        if (dirRow) dirRow = dirRow > 0 ? 1 : -1;
        if (dirCol) dirCol = dirCol > 0 ? 1 : -1;

        glColor3f(0, 0, 0);
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 2 * 3.14159f * k / 20;
            glVertex2f(cx - 3.5f + dirCol * 0.6f + cos(a) * 1.2f,
                       cy + 2.5f + dirRow * 0.6f + sin(a) * 1.2f);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        for (int k = 0; k <= 20; k++) {
            float a = 2 * 3.14159f * k / 20;
            glVertex2f(cx + 3.5f + dirCol * 0.6f + cos(a) * 1.2f,
                       cy + 2.5f + dirRow * 0.6f + sin(a) * 1.2f);
        }
        glEnd();
    }

    /* Пакман */
    {
        float cx = offsetX + game->pacman.col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - game->pacman.row) * CELL_SIZE + CELL_SIZE / 2;
        glColor3f(1, 1, 0);
        float mouth = 30.0f, start = 0, end = 360;
        if (game->pacman.dCol == 1)       { start = mouth;         end = 360 - mouth; }
        else if (game->pacman.dCol == -1) { start = 180 + mouth;   end = 540 - mouth; }
        else if (game->pacman.dRow == -1) { start = 90 + mouth;    end = 450 - mouth; }
        else if (game->pacman.dRow == 1)  { start = 270 + mouth;   end = 630 - mouth; }
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (float a = start; a <= end; a += 5) {
            float rad = a * 3.14159f / 180;
            glVertex2f(cx + cos(rad) * 10, cy + sin(rad) * 10);
        }
        glEnd();
    }

    /* HUD */
    glColor3f(1, 1, 1);
    drawString(50, 620, 12, "SCORE");
    drawNumber(50, 590, 12, game->score);
    drawString(180, 620, 12, "LIVES");
    drawDigit(180, 590, 12, game->lives);
    drawString(310, 620, 12, "DOTS");
    drawNumber(310, 590, 12, game->dotsLeft);
    drawString(450, 620, 12, "BEST");
    drawNumber(450, 590, 12, numScores > 0 ? highScores[0] : 0);
    if (game->frightened) drawString(570, 620, 12, "FRIGHT");

    if (game->gameOver) {
        glColor3f(0.2f, 0, 0);
        glBegin(GL_QUADS);
        glVertex2f(120, 250); glVertex2f(480, 250);
        glVertex2f(480, 370); glVertex2f(120, 370);
        glEnd();
        glColor3f(1, 1, 1);
        drawString(190, 350, 18, "GAME OVER");
        drawString(170, 310, 12, "PRESS R TO RESTART");
        drawString(170, 280, 12, "OR ESC FOR MAIN MENU");
    } else if (game->win) {
        glColor3f(0, 0.2f, 0);
        glBegin(GL_QUADS);
        glVertex2f(120, 250); glVertex2f(480, 250);
        glVertex2f(480, 370); glVertex2f(120, 370);
        glEnd();
        glColor3f(1, 1, 1);
        drawString(220, 350, 18, "YOU WIN");
        drawString(170, 310, 12, "PRESS R TO RESTART");
        drawString(170, 280, 12, "OR ESC FOR MAIN MENU");
    }
}
