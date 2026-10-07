#include "menu.h"
#include "render.h"
#include "highscores.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

const char* menuItems[] = { "START GAME", "HELP", "ABOUT", "EXIT" };
int menuSize = 4;
int selectedMenuItem = 0;

void drawMainMenu(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(1, 1, 0);
    drawString(280, 580, 28, "  PACMAN");

    glColor3f(1, 1, 1);
    drawString(200, 520, 16, "    BEST SCORE: ");
    char buf[20];
    if (numScores > 0) sprintf(buf, "%d", highScores[0]);
    else               sprintf(buf, "0");
    drawString(430, 520, 16, buf);

    for (int i = 0; i < menuSize; i++) {
        if (i == selectedMenuItem) glColor3f(1, 0.8f, 0);
        else                       glColor3f(1, 1, 1);
        drawString(320, 450 - i * 55, 18, menuItems[i]);
    }

    glColor3f(0.6f, 0.6f, 0.6f);
    drawString(170, 140, 12, "   USE WASD OR ARROWS, ENTER TO SELECT");
}

void drawHelp(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glColor3f(1, 1, 1);
    drawString(330, 620, 22, "HELP");
    drawString(100, 560, 14, "CONTROLS:");
    drawString(100, 520, 13, "W or UP ARROW - MOVE UP");
    drawString(100, 485, 13, "S or DOWN ARROW - MOVE DOWN");
    drawString(100, 450, 13, "A or LEFT ARROW - MOVE LEFT");
    drawString(100, 415, 13, "D or RIGHT ARROW - MOVE RIGHT");
    drawString(100, 370, 13, "R - RESTART (WHEN GAME OVER)");
    drawString(100, 335, 13, "ESC - RETURN TO MAIN MENU");
    drawString(100, 290, 14, "GAME RULES:");
    drawString(100, 255, 12, "EAT ALL DOTS TO WIN. AVOID GHOSTS OR KILL");
    drawString(100, 225, 12, "THEM WHEN THEY ARE FRIGHTENED AFTER EATING");
    drawString(100, 195, 12, "A BIG DOT. EACH GHOST GIVES 200 POINTS");
    glColor3f(0.7f, 0.7f, 0.7f);
    drawString(100, 130, 12, "PRESS ESC TO RETURN TO MAIN MENU");
}

void drawAbout(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glColor3f(1, 1, 1);
    drawString(330, 620, 22, "ABOUT");
    drawString(120, 560, 15, "AUTHORS:");
    drawString(120, 520, 13, "USOLTSEV GLEB");
    drawString(120, 485, 13, "TUSYUK MAXIM");
    drawString(120, 440, 15, "GROUP: 5131001-50602");
    drawString(120, 395, 15, "YEAR: 2026");
    drawString(120, 350, 15, "UNIVERSITY: SPBPU");
    drawString(120, 305, 15, "INSTITUTE: IKNT");
    drawString(120, 260, 15, "DEPARTMENT: CYBERSECURITY");
    glColor3f(0.7f, 0.7f, 0.7f);
    drawString(120, 150, 12, "PRESS ESC TO RETURN TO MAIN MENU");
}
