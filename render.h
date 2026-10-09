#ifndef RENDER_H
#define RENDER_H

#include "game.h"

void initRender(void);
void shutdownRender(void);
void beginFrame(void);
void setColor(float r, float g, float b);
void drawScene(const Game* game);
void drawString(float x, float y, float size, const char* str);
void drawStringGlow(float x, float y, float size, const char* str);
void drawStringCentered(float cx, float y, float size, const char* str);
void drawStringCenteredGlow(float cx, float y, float size, const char* str);
float textWidth(float size, const char* str);
void drawProgressBar(float x, float y, float w, float h,
                     float percent, const char* label);

#endif
