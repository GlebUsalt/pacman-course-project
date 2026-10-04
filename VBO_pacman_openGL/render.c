#include "render.h"
#include "game.h"
#include "highscores.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#define MAX_VERTS 5400

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

#define CIRCLE_VERTS 22
#define PAC_VERTS 62

static GLuint vbo = 0;
static float vertData[MAX_VERTS * 2];
static int vertCount = 0;
static int quadFirst, circleFirst, halfFirst, pacFirst, triFirst, lineFirst;
static int glyphFirst[36], glyphCount[36];

static void addVertex(float x, float y) {
    vertData[vertCount * 2]     = x;
    vertData[vertCount * 2 + 1] = y;
    vertCount++;
}

void initRender(void) {
    const float pi = 3.14159f;

    vertCount = 0;

    quadFirst = vertCount;
    addVertex(0, 0); addVertex(1, 0); addVertex(1, 1); addVertex(0, 1);

    circleFirst = vertCount;
    addVertex(0, 0);
    for (int k = 0; k <= 20; k++) {
        float a = 2 * pi * k / 20;
        addVertex(cosf(a), sinf(a));
    }

    halfFirst = vertCount;
    addVertex(0, 0);
    for (int k = 0; k <= 20; k++) {
        float a = pi * k / 20;
        addVertex(cosf(a), sinf(a));
    }

    pacFirst = vertCount;
    addVertex(0, 0);
    for (float a = 30; a <= 330; a += 5) {
        float rad = a * pi / 180;
        addVertex(cosf(rad), sinf(rad));
    }

    triFirst = vertCount;
    addVertex(0.2f, -0.8f); addVertex(0.4f, -0.8f); addVertex(0.3f, -0.6f);

    lineFirst = vertCount;
    addVertex(0, 0); addVertex(1, 1);

    const float cellW = 0.6f / 5.0f;
    const float cellH = 1.0f / 7.0f;
    for (int idx = 0; idx < 36; idx++) {
        glyphFirst[idx] = vertCount;
        for (int row = 0; row < 7; row++) {
            unsigned char mask = 0x10;
            for (int col = 0; col < 5; col++) {
                if (fontData[idx][row] & mask) {
                    addVertex(col * cellW,       -row * cellH);
                    addVertex((col + 1) * cellW, -row * cellH);
                    addVertex((col + 1) * cellW, -(row + 1) * cellH);
                    addVertex(col * cellW,       -(row + 1) * cellH);
                }
                mask >>= 1;
            }
        }
        glyphCount[idx] = vertCount - glyphFirst[idx];
    }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertCount * 2 * sizeof(float),
                 vertData, GL_STATIC_DRAW);
    glVertexPointer(2, GL_FLOAT, 0, (const void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
}

void shutdownRender(void) {
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (vbo) glDeleteBuffers(1, &vbo);
    vbo = 0;
}

static void drawShape(GLenum mode, int first, int count,
                      float x, float y, float sx, float sy) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(sx, sy, 1);
    glDrawArrays(mode, first, count);
    glPopMatrix();
}

static void fillRect(float x, float y, float w, float h) {
    drawShape(GL_QUADS, quadFirst, 4, x, y, w, h);
}

static void outlineRect(float x, float y, float w, float h) {
    drawShape(GL_LINE_LOOP, quadFirst, 4, x, y, w, h);
}

static void fillCircle(float cx, float cy, float r) {
    drawShape(GL_TRIANGLE_FAN, circleFirst, CIRCLE_VERTS, cx, cy, r, r);
}

static void drawChar(float x, float y, float size, char ch) {
    if (ch == ' ') return;

    if (ch == '.') {
        float s = size * 0.15f;
        fillRect(x + size * 0.3f, y - size * 0.75f, s, s);
        return;
    }
    if (ch == ',') {
        drawShape(GL_TRIANGLES, triFirst, 3, x, y, size, size);
        return;
    }
    if (ch == ':') {
        float r = size * 0.1f;
        fillCircle(x + size * 0.3f, y - size * 0.3f, r);
        fillCircle(x + size * 0.3f, y - size * 0.7f, r);
        return;
    }
    if (ch == '/') {
        drawShape(GL_LINES, lineFirst, 2,
                  x + size * 0.1f, y - size * 0.9f, size * 0.8f, size * 0.8f);
        return;
    }
    if (ch == '%') {
        float r = size * 0.12f;
        fillCircle(x + size * 0.22f, y - size * 0.28f, r);
        fillCircle(x + size * 0.55f, y - size * 0.72f, r);
        drawShape(GL_LINES, lineFirst, 2,
                  x + size * 0.15f, y - size * 0.85f, size * 0.47f, size * 0.7f);
        return;
    }
    if (ch == '-') {
        float w = size * 0.6f;
        float h = size * 0.1f;
        fillRect(x, y - size * 0.5f - h, w, h);
        return;
    }
    if (ch == '(' || ch == ')' || ch == '_') return;

    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'Z')) return;

    int idx;
    if (ch >= '0' && ch <= '9') idx = ch - '0';
    else                        idx = ch - 'A' + 10;
    if (idx < 0 || idx >= 36) return;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);
    glDrawArrays(GL_QUADS, glyphFirst[idx], glyphCount[idx]);
    glPopMatrix();
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
    if (s[0]) fillRect(x, y - t, w, t);
    if (s[1]) fillRect(x + w - t, y - h / 2, t, h / 2);
    if (s[2]) fillRect(x + w - t, y - h, t, h / 2);
    if (s[3]) fillRect(x, y - h, w, t);
    if (s[4]) fillRect(x, y - h, t, h / 2);
    if (s[5]) fillRect(x, y - h / 2, t, h / 2);
    if (s[6]) { float mid = y - h / 2 + t / 2;
                fillRect(x, mid - t, w, t); }
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
    fillRect(x, y - h, w, h);

    glColor3f(0.0f, 1.0f, 0.2f);
    fillRect(x, y - h, w * (percent / 100.0f), h);

    glColor3f(1, 1, 1);
    drawString(x + w / 2 - 40, y - h / 2 - 5, 12, label);
    char buf[20];
    sprintf(buf, "%.0f%%", percent);
    drawString(x + w / 2 - 20, y - h / 2 - 25, 12, buf);
}


void drawScene(const Game* game) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    const float offsetX = 100.0f;
    const float offsetY = 50.0f;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float cx = offsetX + c * CELL_SIZE;
            float cy = offsetY + (ROWS - 1 - r) * CELL_SIZE;
            char cell = game->grid[r][c];

            if (cell == WALL) {
                glColor3f(0, 0, 0.8f);
                fillRect(cx, cy, CELL_SIZE, CELL_SIZE);
                glColor3f(1, 1, 1);
                outlineRect(cx, cy, CELL_SIZE, CELL_SIZE);
            } else if (cell == DOT) {
                glColor3f(1, 0.8f, 0.6f);
                fillCircle(cx + CELL_SIZE / 2, cy + CELL_SIZE / 2, 2.5f);
            } else if (cell == ENERGIZER) {
                glColor3f(1, 0.8f, 0.6f);
                fillCircle(cx + CELL_SIZE / 2, cy + CELL_SIZE / 2, 6.0f);
            } else if (cell == PORTAL) {
                float cx2 = cx + CELL_SIZE / 2, cy2 = cy + CELL_SIZE / 2;
                glColor3f(0.8f, 0.2f, 1.0f);
                fillCircle(cx2, cy2, 8.0f);
                glColor3f(0.0f, 0.0f, 0.0f);
                fillCircle(cx2, cy2, 4.0f);
            }
        }
    }

    for (int g = 0; g < NUM_GHOSTS; g++) {
        const Ghost* ghost = &game->ghosts[g];
        float cx = offsetX + ghost->col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - ghost->row) * CELL_SIZE + CELL_SIZE / 2;

        if (game->frightened) glColor3f(0.3f, 0.3f, 1.0f);
        else                  glColor3f(ghost->r, ghost->g, ghost->b);

        drawShape(GL_TRIANGLE_FAN, halfFirst, CIRCLE_VERTS, cx, cy, 9, 9);
        fillRect(cx - 9, cy - 7, 18, 7);

        glColor3f(1, 1, 1);
        fillCircle(cx - 3.5f, cy + 2.5f, 2.5f);
        fillCircle(cx + 3.5f, cy + 2.5f, 2.5f);

        int dirRow = game->pacman.row - ghost->row;
        int dirCol = game->pacman.col - ghost->col;
        if (dirRow) dirRow = dirRow > 0 ? 1 : -1;
        if (dirCol) dirCol = dirCol > 0 ? 1 : -1;

        glColor3f(0, 0, 0);
        fillCircle(cx - 3.5f + dirCol * 0.6f, cy + 2.5f + dirRow * 0.6f, 1.2f);
        fillCircle(cx + 3.5f + dirCol * 0.6f, cy + 2.5f + dirRow * 0.6f, 1.2f);
    }

    {
        float cx = offsetX + game->pacman.col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - game->pacman.row) * CELL_SIZE + CELL_SIZE / 2;
        glColor3f(1, 1, 0);
        float angle = 0;
        bool full = false;
        if (game->pacman.dCol == 1)        angle = 0;
        else if (game->pacman.dCol == -1)  angle = 180;
        else if (game->pacman.dRow == -1)  angle = 90;
        else if (game->pacman.dRow == 1)   angle = 270;
        else                               full = true;

        if (full) {
            fillCircle(cx, cy, 10);
        } else {
            glPushMatrix();
            glTranslatef(cx, cy, 0);
            glRotatef(angle, 0, 0, 1);
            glScalef(10, 10, 1);
            glDrawArrays(GL_TRIANGLE_FAN, pacFirst, PAC_VERTS);
            glPopMatrix();
        }
    }

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
        fillRect(120, 250, 360, 120);
        glColor3f(1, 1, 1);
        drawString(190, 350, 18, "GAME OVER");
        drawString(170, 310, 12, "PRESS R TO RESTART");
        drawString(170, 280, 12, "OR ESC FOR MAIN MENU");
    } else if (game->win) {
        glColor3f(0, 0.2f, 0);
        fillRect(120, 250, 360, 120);
        glColor3f(1, 1, 1);
        drawString(220, 350, 18, "YOU WIN");
        drawString(170, 310, 12, "PRESS R TO RESTART");
        drawString(170, 280, 12, "OR ESC FOR MAIN MENU");
    }
}
