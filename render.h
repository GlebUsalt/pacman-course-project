#ifndef RENDER_H
#define RENDER_H

#include "constants.h"

void drawScene(void);
void drawProgressBar(float x, float y, float w, float h, float percent, const char* label);
void drawString(float x, float y, float size, const char* str);

#endif