#include "render.h"
#include "game.h"
#include "highscores.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#define MAX_VERTS 9000
#define VIEW_W 750.0f
#define VIEW_H 700.0f

/* ---------- Pixel font (5x7), unchanged ---------- */

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

/* ---------- Shaders (OpenGL 3.3 core) ---------- */

static const char* VERT_SRC =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "uniform vec2  uView;\n"       /* logical window size, replaces glOrtho   */
    "uniform vec2  uOffset;\n"     /* replaces glTranslatef                   */
    "uniform vec2  uScale;\n"      /* replaces glScalef                       */
    "uniform float uAngle;\n"      /* radians, replaces glRotatef             */
    "out vec2 vLocal;\n"
    "void main() {\n"
    "    vLocal = aPos;\n"
    "    float c = cos(uAngle), s = sin(uAngle);\n"
    "    vec2 p = aPos * uScale;\n"
    "    p = vec2(c * p.x - s * p.y, s * p.x + c * p.y) + uOffset;\n"
    "    gl_Position = vec4(p / uView * 2.0 - 1.0, 0.0, 1.0);\n"
    "}\n";

/* uMode: 0 flat colour, 1 circle, 2 pac-man, 3 ghost body.
   uParams: x = radius (px), y = half-size of the quad (px),
            z = glow strength, w = mouth half-angle (rad).               */
static const char* FRAG_SRC =
    "#version 330 core\n"
    "in vec2 vLocal;\n"
    "uniform vec3  uColor;\n"
    "uniform int   uMode;\n"
    "uniform vec4  uParams;\n"
    "uniform float uTime;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    if (uMode == 0) { FragColor = vec4(uColor, 1.0); return; }\n"
    "    float pad = uParams.y;\n"
    "    vec2 p = vLocal * pad;\n"
    "    const float aa = 0.75;\n"
    "    if (uMode == 3) {\n"
    "        float yb = -7.0 + 1.5 * sin(p.x * 1.05 + uTime * 8.0);\n"
    "        float d = (p.y >= 0.0) ? length(p) - 9.0\n"
    "                               : max(abs(p.x) - 9.0, yb - p.y);\n"
    "        FragColor = vec4(uColor, 1.0 - smoothstep(-aa, aa, d));\n"
    "        return;\n"
    "    }\n"
    "    float r = uParams.x;\n"
    "    float dist = length(p);\n"
    "    float core = 1.0 - smoothstep(r - aa, r + aa, dist);\n"
    "    if (uMode == 2) {\n"
    "        float m = uParams.w;\n"
    "        vec2 q = vec2(p.x, abs(p.y));\n"
    "        core *= smoothstep(-aa, aa, dot(q, vec2(-sin(m), cos(m))));\n"
    "    }\n"
    "    float halo = 0.0;\n"
    "    if (uParams.z > 0.0 && pad > r && dist > r) {\n"
    "        float t = clamp((dist - r) / (pad - r), 0.0, 1.0);\n"
    "        halo = uParams.z * (1.0 - t) * (1.0 - t);\n"
    "    }\n"
    "    FragColor = vec4(uColor, max(core, halo));\n"
    "}\n";

enum { MODE_FLAT = 0, MODE_CIRCLE = 1, MODE_PACMAN = 2, MODE_GHOST = 3 };

static GLuint prog = 0, vao = 0, vbo = 0;
static GLint uView, uOffset, uScale, uAngle, uColor, uMode, uParams, uTime;

/* ---------- Geometry (triangles only: GL_QUADS is gone in core) ---------- */

static float vertData[MAX_VERTS * 2];
static int vertCount = 0;
static int quadFirst, loopFirst, centerQuadFirst, triFirst, lineFirst;
static int glyphFirst[36], glyphCount[36];

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

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error (%s):\n%s\n",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
    }
    return s;
}

static void buildProgram(void) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Shader link error:\n%s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    uView   = glGetUniformLocation(prog, "uView");
    uOffset = glGetUniformLocation(prog, "uOffset");
    uScale  = glGetUniformLocation(prog, "uScale");
    uAngle  = glGetUniformLocation(prog, "uAngle");
    uColor  = glGetUniformLocation(prog, "uColor");
    uMode   = glGetUniformLocation(prog, "uMode");
    uParams = glGetUniformLocation(prog, "uParams");
    uTime   = glGetUniformLocation(prog, "uTime");
}

void initRender(void) {
    vertCount = 0;

    quadFirst = vertCount;                       /* unit square, 2 triangles  */
    addQuad(0, 0, 1, 1);

    loopFirst = vertCount;                       /* unit square outline       */
    addVertex(0, 0); addVertex(1, 0); addVertex(1, 1); addVertex(0, 1);

    centerQuadFirst = vertCount;                 /* [-1,1] square for shaders */
    addQuad(-1, -1, 1, 1);

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
                if (fontData[idx][row] & mask)
                    addQuad(col * cellW, -(row + 1) * cellH,
                            (col + 1) * cellW, -row * cellH);
                mask >>= 1;
            }
        }
        glyphCount[idx] = vertCount - glyphFirst[idx];
    }

    buildProgram();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertCount * 2 * sizeof(float),
                 vertData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 1);
}

void shutdownRender(void) {
    if (vbo)  glDeleteBuffers(1, &vbo);
    if (vao)  glDeleteVertexArrays(1, &vao);
    if (prog) glDeleteProgram(prog);
    vbo = vao = prog = 0;
}

void beginFrame(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog);
    glBindVertexArray(vao);
    glUniform2f(uView, VIEW_W, VIEW_H);
    glUniform1f(uTime, (float)glfwGetTime());
    glUniform1f(uAngle, 0.0f);
    glUniform1i(uMode, MODE_FLAT);
    glUniform3f(uColor, 1, 1, 1);
}

void setColor(float r, float g, float b) {
    glUniform3f(uColor, r, g, b);
}

/* ---------- Drawing helpers ---------- */

static void drawShape(GLenum mode, int first, int count,
                      float x, float y, float sx, float sy) {
    glUniform1i(uMode, MODE_FLAT);
    glUniform1f(uAngle, 0.0f);
    glUniform2f(uOffset, x, y);
    glUniform2f(uScale, sx, sy);
    glDrawArrays(mode, first, count);
}

static void fillRect(float x, float y, float w, float h) {
    drawShape(GL_TRIANGLES, quadFirst, 6, x, y, w, h);
}

static void outlineRect(float x, float y, float w, float h) {
    drawShape(GL_LINE_LOOP, loopFirst, 4, x, y, w, h);
}

/* Draws one of the shader-made shapes inside a square of half-size `pad`. */
static void drawSdf(int mode, float cx, float cy, float pad, float angle,
                    float radius, float glow, float mouth) {
    glUniform1i(uMode, mode);
    glUniform4f(uParams, radius, pad, glow, mouth);
    glUniform2f(uOffset, cx, cy);
    glUniform2f(uScale, pad, pad);
    glUniform1f(uAngle, angle);
    glDrawArrays(GL_TRIANGLES, centerQuadFirst, 6);
    glUniform1i(uMode, MODE_FLAT);
    glUniform1f(uAngle, 0.0f);
}

static void fillCircle(float cx, float cy, float r) {
    drawSdf(MODE_CIRCLE, cx, cy, r + 1.5f, 0.0f, r, 0.0f, 0.0f);
}

static void glowCircle(float cx, float cy, float r, float haloPad, float glow) {
    drawSdf(MODE_CIRCLE, cx, cy, haloPad, 0.0f, r, glow, 0.0f);
}

/* ---------- Text ---------- */

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

    drawShape(GL_TRIANGLES, glyphFirst[idx], glyphCount[idx], x, y, size, size);
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
    setColor(0.2f, 0.2f, 0.6f);
    fillRect(x, y - h, w, h);

    setColor(0.0f, 1.0f, 0.2f);
    fillRect(x, y - h, w * (percent / 100.0f), h);

    setColor(1, 1, 1);
    drawString(x + w / 2 - 40, y - h / 2 - 5, 12, label);
    char buf[20];
    sprintf(buf, "%.0f%%", percent);
    drawString(x + w / 2 - 20, y - h / 2 - 25, 12, buf);
}

/* ---------- Scene ---------- */

void drawScene(const Game* game) {
    beginFrame();
    const float offsetX = 100.0f;
    const float offsetY = 50.0f;
    const float t = (float)glfwGetTime();
    const float deg = 3.14159265f / 180.0f;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float cx = offsetX + c * CELL_SIZE;
            float cy = offsetY + (ROWS - 1 - r) * CELL_SIZE;
            float mx = cx + CELL_SIZE / 2, my = cy + CELL_SIZE / 2;
            char cell = game->grid[r][c];

            if (cell == WALL) {
                setColor(0, 0, 0.8f);
                fillRect(cx, cy, CELL_SIZE, CELL_SIZE);
                setColor(1, 1, 1);
                outlineRect(cx, cy, CELL_SIZE, CELL_SIZE);
            } else if (cell == DOT) {
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

    {
        float cx = offsetX + game->pacman.col * CELL_SIZE + CELL_SIZE / 2;
        float cy = offsetY + (ROWS - 1 - game->pacman.row) * CELL_SIZE + CELL_SIZE / 2;
        float angle = 0;
        bool full = false;
        if (game->pacman.dCol == 1)        angle = 0;
        else if (game->pacman.dCol == -1)  angle = 180;
        else if (game->pacman.dRow == -1)  angle = 90;
        else if (game->pacman.dRow == 1)   angle = 270;
        else                               full = true;

        /* mouth opens and closes between 6 and 38 degrees */
        float mouth = full ? 0.0f
                           : (22.0f + 16.0f * sinf(t * 14.0f)) * deg;
        setColor(1, 1, 0);
        drawSdf(MODE_PACMAN, cx, cy, 16.0f, angle * deg, 10.0f, 0.3f, mouth);
    }

    setColor(1, 1, 1);
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
        setColor(0.2f, 0, 0);
        fillRect(120, 250, 360, 120);
        setColor(1, 1, 1);
        drawString(190, 350, 18, "GAME OVER");
        drawString(170, 310, 12, "PRESS R TO RESTART");
        drawString(170, 280, 12, "OR ESC FOR MAIN MENU");
    } else if (game->win) {
        setColor(0, 0.2f, 0);
        fillRect(120, 250, 360, 120);
        setColor(1, 1, 1);
        drawString(220, 350, 18, "YOU WIN");
        drawString(170, 310, 12, "PRESS R TO RESTART");
        drawString(170, 280, 12, "OR ESC FOR MAIN MENU");
    }
}
