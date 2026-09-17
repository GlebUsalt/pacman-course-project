#ifndef RENDER_H
#define RENDER_H

#include "game.h"

void drawScene(const Game* game);
void drawString(float x, float y, float size, const char* str);
void drawProgressBar(float x, float y, float w, float h,
                     float percent, const char* label);

#endif
