#include "menu.h"
#include "render.h"
#include "highscores.h"
#include <stdio.h>

#define CX 375.0f                              /* centre of the 750 px window */

const char* menuItems[] = { "START GAME", "HELP", "ABOUT", "EXIT" };
int menuSize = 4;
int selectedMenuItem = 0;

void drawMainMenu(void) {
    beginFrame();

    setColor(1, 0.9f, 0);
    drawStringCenteredGlow(CX, 640, 48, "PACMAN");

    char buf[40];
    sprintf(buf, "BEST SCORE: %d", numScores > 0 ? highScores[0] : 0);
    setColor(0.5f, 0.75f, 1.0f);
    drawStringCentered(CX, 525, 18, buf);

    for (int i = 0; i < menuSize; i++) {
        float y = 440.0f - i * 60.0f;
        if (i == selectedMenuItem) {
            char sel[32];
            sprintf(sel, "> %s <", menuItems[i]);
            setColor(1, 0.85f, 0.1f);
            drawStringCenteredGlow(CX, y, 26, sel);
        } else {
            setColor(0.85f, 0.9f, 1.0f);
            drawStringCentered(CX, y, 24, menuItems[i]);
        }
    }

    setColor(0.55f, 0.6f, 0.75f);
    drawStringCentered(CX, 80, 13, "USE W S OR ARROWS, ENTER TO SELECT");
}

void drawHelp(void) {
    beginFrame();
    setColor(0.4f, 0.8f, 1.0f);
    drawStringCenteredGlow(CX, 670, 32, "HELP");

    setColor(1, 0.85f, 0.1f);
    drawString(70, 590, 17, "CONTROLS");
    setColor(1, 1, 1);
    drawString(70, 550, 14, "W OR UP ARROW - MOVE UP");
    drawString(70, 516, 14, "S OR DOWN ARROW - MOVE DOWN");
    drawString(70, 482, 14, "A OR LEFT ARROW - MOVE LEFT");
    drawString(70, 448, 14, "D OR RIGHT ARROW - MOVE RIGHT");
    drawString(70, 408, 14, "R - RESTART WHEN THE GAME IS OVER");
    drawString(70, 374, 14, "ESC - RETURN TO MAIN MENU");

    setColor(1, 0.85f, 0.1f);
    drawString(70, 322, 17, "GAME RULES");
    setColor(1, 1, 1);
    drawString(70, 282, 14, "EAT ALL DOTS TO WIN. AVOID THE GHOSTS,");
    drawString(70, 252, 14, "OR EAT A BIG DOT AND CATCH THEM WHILE");
    drawString(70, 222, 14, "THEY ARE FRIGHTENED. EACH GHOST GIVES");
    drawString(70, 192, 14, "200 POINTS.");

    setColor(0.55f, 0.6f, 0.75f);
    drawStringCentered(CX, 80, 13, "PRESS ESC TO RETURN TO MAIN MENU");
}

void drawAbout(void) {
    beginFrame();
    setColor(0.4f, 0.8f, 1.0f);
    drawStringCenteredGlow(CX, 670, 32, "ABOUT");

    setColor(1, 0.85f, 0.1f);
    drawStringCentered(CX, 580, 18, "AUTHORS");
    setColor(1, 1, 1);
    drawStringCentered(CX, 540, 18, "USOLTSEV GLEB");
    drawStringCentered(CX, 505, 18, "TUSYUK MAXIM");

    setColor(0.85f, 0.9f, 1.0f);
    drawStringCentered(CX, 440, 16, "GROUP: 5131001-50602");
    drawStringCentered(CX, 400, 16, "YEAR: 2026");
    drawStringCentered(CX, 360, 16, "UNIVERSITY: SPBPU");
    drawStringCentered(CX, 320, 16, "INSTITUTE: IKNT");
    drawStringCentered(CX, 280, 16, "DEPARTMENT: CYBERSECURITY");

    setColor(0.55f, 0.6f, 0.75f);
    drawStringCentered(CX, 80, 13, "PRESS ESC TO RETURN TO MAIN MENU");
}
