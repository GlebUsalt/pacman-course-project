#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>

// Режимы, по которым фрагментный шейдер выбирает, что рисовать (uMode)
enum { MODE_FLAT = 0, MODE_CIRCLE = 1, MODE_PACMAN = 2, MODE_GHOST = 3,
       MODE_WALL = 4, MODE_CAPSULE = 5 };

extern GLuint shaderProg;                      // готовая шейдерная программа
// "Пульты управления" шейдером: через них C-код передаёт данные видеокарте
extern GLint uView, uOffset, uScale, uAngle, uColor, uMode, uParams, uTime, uAlpha;

void buildProgram(void);                       // компилирует шейдеры и находит все uniform
void destroyProgram(void);                     // удаляет программу при выходе

#endif
