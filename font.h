#ifndef FONT_H
#define FONT_H

#define MAX_SEGS 16                            // максимум отрезков в одной букве

typedef struct {
    int   n;                                   // сколько отрезков в букве
    float seg[MAX_SEGS][4];                    // отрезки: x1, y1, x2, y2 на сетке 4x6
} Glyph;

void initFont(void);                           // разбирает таблицу букв, вызывать один раз при старте
const Glyph* getGlyph(unsigned char ch);       // рисунок буквы или NULL, если такой буквы нет

#endif
