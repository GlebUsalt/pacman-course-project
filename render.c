#include "render.h"
#include "game.h"
#include "highscores.h"
#include "font.h"
#include "shaders.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#define MAX_VERTS 64
#define VIEW_W 750.0f                          //логическая ширина окна в пикселях
#define VIEW_H 700.0f                          //логическая высота окна

#define WALL_MARGIN 7.0f                       //запас вокруг клетки стены под свечение

static GLuint vao = 0, vbo = 0;                //объекты видеокарты с геометрией

static float vertData[MAX_VERTS * 2];
static int vertCount = 0;
static int quadFirst, centerQuadFirst;

static void addVertex(float x, float y) {
    if (vertCount >= MAX_VERTS) return;
    vertData[vertCount * 2]     = x;
    vertData[vertCount * 2 + 1] = y;
    vertCount++;
}

static void addQuad(float x0, float y0, float x1, float y1) {
    addVertex(x0, y0); addVertex(x1, y0); addVertex(x1, y1);
    addVertex(x0, y0); addVertex(x1, y1); addVertex(x0, y1);
}

void initRender(void) {
    vertCount = 0;

    quadFirst = vertCount;                       //единичный квадрат из 2 треугольников - для прямоугольников
    addQuad(0, 0, 1, 1);

    centerQuadFirst = vertCount;                 //квадрат [-1,1] - шейдер вырезает из него круги, стены, буквы
    addQuad(-1, -1, 1, 1);

    initFont();                                //готовим рисунки букв

    buildProgram();                            //компилируем шейдеры (код в shaders.c)

    glGenVertexArrays(1, &vao);                //VAO/VBO - так вершины квадратов попадают на видеокарту
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertCount * 2 * sizeof(float),
                 vertData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_BLEND);                        //включаем прозрачность, без неё не будет свечения
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 1);
}

void shutdownRender(void) {
    if (vbo)  glDeleteBuffers(1, &vbo);
    if (vao)  glDeleteVertexArrays(1, &vao);
    destroyProgram();
    vbo = vao = 0;
}

void beginFrame(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProg);
    glBindVertexArray(vao);
    glUniform2f(uView, VIEW_W, VIEW_H);
    glUniform1f(uTime, (float)glfwGetTime());
    glUniform1f(uAngle, 0.0f);
    glUniform1f(uAlpha, 1.0f);
    glUniform1i(uMode, MODE_FLAT);
    glUniform3f(uColor, 1, 1, 1);
}

void setColor(float r, float g, float b) {
    glUniform3f(uColor, r, g, b);
}

static void setAlpha(float a) {
    glUniform1f(uAlpha, a);
}

static void fillRect(float x, float y, float w, float h) {
    glUniform1i(uMode, MODE_FLAT);
    glUniform1f(uAngle, 0.0f);
    glUniform2f(uOffset, x, y);
    glUniform2f(uScale, w, h);
    glDrawArrays(GL_TRIANGLES, quadFirst, 6);
}

//Рисует фигуру из шейдера (круг, Pac-Man, призрак, стена) внутри квадрата с половиной стороны pad
static void drawSdf(int mode, float cx, float cy, float pad, float angle,
                    float radius, float glow, float mouth) {
    glUniform1i(uMode, mode);
    glUniform4f(uParams, radius, pad, glow, mouth);   //все параметры фигуры одним пакетом для шейдера
    glUniform2f(uOffset, cx, cy);
    glUniform2f(uScale, pad, pad);
    glUniform1f(uAngle, angle);
    glDrawArrays(GL_TRIANGLES, centerQuadFirst, 6);
    glUniform1i(uMode, MODE_FLAT);             //возвращаем обычный режим, чтобы не сломать следующий вызов
    glUniform1f(uAngle, 0.0f);
}

static void fillCircle(float cx, float cy, float r) {
    drawSdf(MODE_CIRCLE, cx, cy, r + 1.5f, 0.0f, r, 0.0f, 0.0f);
}

static void glowCircle(float cx, float cy, float r, float haloPad, float glow) {
    drawSdf(MODE_CIRCLE, cx, cy, haloPad, 0.0f, r, glow, 0.0f);
}

//Отрезок с круглыми концами и толщиной r: из таких отрезков состоят буквы и рамки
static void drawCapsule(float ax, float ay, float bx, float by, float r) {
    float dx = bx - ax, dy = by - ay;
    float half = 0.5f * sqrtf(dx * dx + dy * dy);   //половина длины отрезка
    float pad = r + 1.5f;
    glUniform1i(uMode, MODE_CAPSULE);
    glUniform4f(uParams, half, pad, r, half + pad);
    glUniform2f(uOffset, 0.5f * (ax + bx), 0.5f * (ay + by));
    glUniform2f(uScale, half + pad, pad);
    glUniform1f(uAngle, atan2f(dy, dx));        //поворачиваем квадрат вдоль отрезка
    glDrawArrays(GL_TRIANGLES, centerQuadFirst, 6);
    glUniform1i(uMode, MODE_FLAT);
    glUniform1f(uAngle, 0.0f);
}

static void strokeRect(float x, float y, float w, float h, float r) {
    drawCapsule(x,     y,     x + w, y,     r);
    drawCapsule(x + w, y,     x + w, y + h, r);
    drawCapsule(x + w, y + h, x,     y + h, r);
    drawCapsule(x,     y + h, x,     y,     r);
}

// Рамка со свечением: сначала широкая бледная линия, поверх неё тонкая яркая
static void neonRect(float x, float y, float w, float h) {
    setAlpha(0.18f);
    strokeRect(x, y, w, h, 5.0f);
    setAlpha(1.0f);
    strokeRect(x, y, w, h, 1.6f);
}

float textWidth(float size, const char* str) {
    int n = 0;
    while (str[n]) n++;
    if (n == 0) return 0.0f;
    float u = size / 6.0f;
    return (n - 1) * 5.4f * u + 4.0f * u;
}

static void drawText(float x, float y, float size, const char* str,
                     float thickness, float alpha) {
    float u = size / 6.0f;
    float r = fmaxf(0.6f, u * 0.3f) * thickness;
    float bottom = y - size;
    setAlpha(alpha);
    for (; *str; str++) {
        unsigned char ch = (unsigned char)*str;
        if (ch >= 'a' && ch <= 'z') ch = (unsigned char)(ch - 'a' + 'A');
        const Glyph* g = getGlyph(ch);                 //рисунок буквы из font.c
        if (g) {
            for (int i = 0; i < g->n; i++)
                drawCapsule(x + g->seg[i][0] * u, bottom + g->seg[i][1] * u,
                            x + g->seg[i][2] * u, bottom + g->seg[i][3] * u, r);
        }
        x += 5.4f * u;
    }
    setAlpha(1.0f);
}

void drawString(float x, float y, float size, const char* str) {
    drawText(x, y, size, str, 1.0f, 1.0f);
}

void drawStringGlow(float x, float y, float size, const char* str) {
    drawText(x, y, size, str, 1.0f, 1.0f);
}

void drawStringCentered(float cx, float y, float size, const char* str) {
    drawString(cx - textWidth(size, str) * 0.5f, y, size, str);
}

void drawStringCenteredGlow(float cx, float y, float size, const char* str) {
    drawStringGlow(cx - textWidth(size, str) * 0.5f, y, size, str);
}

void drawProgressBar(float x, float y, float w, float h,
                     float percent, const char* label) {
    float cx = x + w * 0.5f;

    setColor(0.3f, 0.8f, 1.0f);
    drawStringCenteredGlow(cx, y + 50, 22, label);

    setColor(0.04f, 0.07f, 0.18f);
    fillRect(x, y - h, w, h);
    setColor(0.1f, 0.9f, 1.0f);
    fillRect(x + 3, y - h + 3, (w - 6) * (percent / 100.0f), h - 6);
    setColor(0.2f, 0.6f, 1.0f);
    neonRect(x, y - h, w, h);

    char buf[20];
    sprintf(buf, "%.0f%%", percent);
    setColor(1, 1, 1);
    drawStringCentered(cx, y - h - 22, 20, buf);
}

static bool isWallAt(const Game* game, int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return true;
    return game->grid[r][c] == WALL;
}

void drawScene(const Game* game) {
    beginFrame();
    const float offsetX = (VIEW_W - COLS * CELL_SIZE) * 0.5f;
    const float offsetY = 50.0f;
    const float t = (float)glfwGetTime();
    const float deg = 3.14159265f / 180.0f;
    const float centerX = VIEW_W * 0.5f;

    setColor(0.15f, 0.5f, 1.0f);
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (game->grid[r][c] != WALL) continue;
            float mx = offsetX + c * CELL_SIZE + CELL_SIZE / 2;
            float my = offsetY + (ROWS - 1 - r) * CELL_SIZE + CELL_SIZE / 2;
            int mask = (isWallAt(game, r - 1, c) ? 1 : 0) |   //маска соседей-стен: 1 сверху, 2 снизу, 4 справа, 8 слева
                       (isWallAt(game, r + 1, c) ? 2 : 0) |
                       (isWallAt(game, r, c + 1) ? 4 : 0) |
                       (isWallAt(game, r, c - 1) ? 8 : 0);
            if (mask == 15) continue;                   //стена окружена стенами - светить нечему
            drawSdf(MODE_WALL, mx, my, CELL_SIZE / 2 + WALL_MARGIN, 0.0f,
                    (float)mask, 0.0f, 0.0f);
        }
    }

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float cx = offsetX + c * CELL_SIZE;
            float cy = offsetY + (ROWS - 1 - r) * CELL_SIZE;
            float mx = cx + CELL_SIZE / 2, my = cy + CELL_SIZE / 2;
            char cell = game->grid[r][c];

            if (cell == DOT) {
                setColor(1, 0.8f, 0.6f);
                fillCircle(mx, my, 2.5f);
            } else if (cell == ENERGIZER) {
                float pulse = 0.5f + 0.5f * sinf(t * 5.0f);
                setColor(1, 0.8f, 0.6f);
                glowCircle(mx, my, 5.0f + 1.0f * pulse, 13.0f,
                           0.25f + 0.35f * pulse);
            } else if (cell == PORTAL) {
                float pulse = 0.5f + 0.5f * sinf(t * 3.0f);
                setColor(0.8f, 0.2f, 1.0f);
                glowCircle(mx, my, 8.0f, 14.0f, 0.3f + 0.3f * pulse);
                setColor(0.0f, 0.0f, 0.0f);
                fillCircle(mx, my, 4.0f);
            }
        }
    }

    const bool flash = game->frightened && game->frightTimer < 100 &&
                       ((int)(t * 5.0f) % 2 == 0);

    for (int g = 0; g < NUM_GHOSTS; g++) {
        const Ghost* ghost = &game->ghosts[g];
        float cx = offsetX + ghost->col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - ghost->row) * CELL_SIZE + CELL_SIZE / 2;

        if (game->frightened) {
            if (flash) setColor(1.0f, 1.0f, 1.0f);
            else       setColor(0.3f, 0.3f, 1.0f);
        } else {
            setColor(ghost->r, ghost->g, ghost->b);
        }
        drawSdf(MODE_GHOST, cx, cy, 11.0f, 0.0f, 0.0f, 0.0f, 0.0f);

        setColor(1, 1, 1);
        fillCircle(cx - 3.5f, cy + 2.5f, 2.5f);
        fillCircle(cx + 3.5f, cy + 2.5f, 2.5f);

        int dirRow = game->pacman.row - ghost->row;
        int dirCol = game->pacman.col - ghost->col;
        if (dirRow) dirRow = dirRow > 0 ? 1 : -1;
        if (dirCol) dirCol = dirCol > 0 ? 1 : -1;

        setColor(0, 0, 0);
        fillCircle(cx - 3.5f + dirCol * 0.6f, cy + 2.5f + dirRow * 0.6f, 1.2f);
        fillCircle(cx + 3.5f + dirCol * 0.6f, cy + 2.5f + dirRow * 0.6f, 1.2f);
    }

    static bool wasDead = false;               //запоминаем момент смерти, чтобы анимация шла от нуля
    static float deathStart = 0.0f;
    float deathK = 0.0f;
    if (game->gameOver) {
        if (!wasDead) { wasDead = true; deathStart = t; }
        deathK = (t - deathStart) / 0.9f;       //прогресс анимации: 0 - только умер, 1 - исчез (0.9 с)
    } else {
        wasDead = false;
    }

    if (deathK < 1.0f) {
        float cx = offsetX + game->pacman.col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - game->pacman.row) * CELL_SIZE + CELL_SIZE / 2;
        float angle = 0;
        bool full = false;
        if (game->pacman.dCol == 1)        angle = 0;
        else if (game->pacman.dCol == -1)  angle = 180;
        else if (game->pacman.dRow == -1)  angle = 90;
        else if (game->pacman.dRow == 1)   angle = 270;
        else                               full = true;

        float mouth;
        if (game->gameOver)
            mouth = (22.0f + 158.0f * deathK) * deg;   //рот раскрывается до 180 градусов, и Pac-Man пропадает
        else if (full)
            mouth = 0.0f;
        else
            mouth = (22.0f + 16.0f * sinf(t * 14.0f)) * deg;
        setColor(1, 1, 0);
        drawSdf(MODE_PACMAN, cx, cy, 16.0f, angle * deg, 10.0f,
                0.3f * (1.0f - deathK), mouth);
    }

    const float labelY = 685.0f, valueY = 660.0f;
    const float colX[4] = { centerX - 255, centerX - 85, centerX + 85, centerX + 255 };
    char buf[24];

    setColor(0.45f, 0.7f, 1.0f);
    drawStringCentered(colX[0], labelY, 13, "SCORE");
    drawStringCentered(colX[1], labelY, 13, "LIVES");
    drawStringCentered(colX[2], labelY, 13, "DOTS");
    drawStringCentered(colX[3], labelY, 13, "BEST");

    setColor(1, 1, 1);
    sprintf(buf, "%d", game->score);
    drawStringCentered(colX[0], valueY, 22, buf);
    sprintf(buf, "%d", game->dotsLeft);
    drawStringCentered(colX[2], valueY, 22, buf);
    setColor(1, 0.85f, 0.2f);
    sprintf(buf, "%d", numScores > 0 ? highScores[0] : 0);
    drawStringCentered(colX[3], valueY, 22, buf);

    setColor(1, 1, 0);
    for (int i = 0; i < game->lives; i++) {
        float ix = colX[1] + (i - (game->lives - 1) * 0.5f) * 30.0f;
        drawSdf(MODE_PACMAN, ix, valueY - 14.0f, 15.0f, 0.0f, 11.0f, 0.0f,
                25.0f * deg);
    }


    if (game->gameOver || game->win) {
        const bool lost = game->gameOver;
        setColor(0, 0, 0);
        setAlpha(0.6f);
        fillRect(0, 0, VIEW_W, VIEW_H);
        setAlpha(0.92f);
        if (lost) setColor(0.10f, 0.0f, 0.02f);
        else      setColor(0.0f, 0.08f, 0.03f);
        fillRect(centerX - 230, 235, 460, 240);
        setAlpha(1.0f);

        if (lost) setColor(1.0f, 0.2f, 0.25f);
        else      setColor(0.2f, 1.0f, 0.4f);
        neonRect(centerX - 230, 235, 460, 240);
        drawStringCenteredGlow(centerX, 440, 36, lost ? "GAME OVER" : "YOU WIN");

        setColor(1, 1, 1);
        sprintf(buf, "SCORE: %d", game->score);
        drawStringCentered(centerX, 370, 20, buf);

        setColor(1, 0.85f, 0.2f);
        drawStringCentered(centerX, 320, 16, "PRESS R TO RESTART");
        setColor(0.7f, 0.8f, 1.0f);
        drawStringCentered(centerX, 288, 16, "ESC - MAIN MENU");
    }
}
