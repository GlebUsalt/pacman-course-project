#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "constants.h"
#include "game.h"
#include "render.h"
#include "menu.h"
#include "highscores.h"

GameState currentState = STATE_MAIN_MENU;
int selectedMenuItem = 0;
const char* menuItems[] = { "START GAME", "HELP", "ABOUT", "EXIT" };
int menuSize = 4;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (currentState == STATE_MAIN_MENU) {
        switch (key) {
        case GLFW_KEY_UP: case GLFW_KEY_W:
            selectedMenuItem = (selectedMenuItem - 1 + menuSize) % menuSize;
            break;
        case GLFW_KEY_DOWN: case GLFW_KEY_S:
            selectedMenuItem = (selectedMenuItem + 1) % menuSize;
            break;
        case GLFW_KEY_ENTER: case GLFW_KEY_SPACE:
            if (selectedMenuItem == 0) {
                currentState = STATE_PLAYING;
                initGame();
                lastUpdateTime = glfwGetTime();
            }
            else if (selectedMenuItem == 1) {
                currentState = STATE_HELP;
            }
            else if (selectedMenuItem == 2) {
                currentState = STATE_ABOUT;
            }
            else if (selectedMenuItem == 3) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
    else if (currentState == STATE_PLAYING) {
        if (key == GLFW_KEY_ESCAPE) {
            currentState = STATE_MAIN_MENU;
            return;
        }
        if (!gameOver && !win) {
            switch (key) {
            case GLFW_KEY_LEFT: case GLFW_KEY_A: pacman.nextDx = 0; pacman.nextDy = -1; break;
            case GLFW_KEY_RIGHT: case GLFW_KEY_D: pacman.nextDx = 0; pacman.nextDy = 1; break;
            case GLFW_KEY_UP: case GLFW_KEY_W: pacman.nextDx = -1; pacman.nextDy = 0; break;
            case GLFW_KEY_DOWN: case GLFW_KEY_S: pacman.nextDx = 1; pacman.nextDy = 0; break;
            }
        }
        if ((gameOver || win) && key == GLFW_KEY_R) {
            initGame();
            gameOver = false;
            win = false;
            frightened = false;
            frightTimer = 0;
            globalTick = 0;
            lastUpdateTime = glfwGetTime();
        }
    }
    else if (currentState == STATE_HELP || currentState == STATE_ABOUT) {
        if (key == GLFW_KEY_ESCAPE) currentState = STATE_MAIN_MENU;
    }
}

void simulateLoading(GLFWwindow* window) {
    for (int p = 0; p <= 100; p++) {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        drawProgressBar(200, 400, 350, 40, (float)p, "LOADING GAME");
        glfwSwapBuffers(window);
        glfwPollEvents();
        double start = glfwGetTime();
        while (glfwGetTime() - start < 0.003) {}
    }
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(750, 700, "Pac-Man 23x21", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glewInit();
    glViewport(0, 0, 750, 700);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 750, 0, 700, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    loadHighScores();
    simulateLoading(window);
    initGame();

    while (!glfwWindowShouldClose(window)) {
        double curr = glfwGetTime(), elapsed = curr - lastUpdateTime;
        if (currentState == STATE_PLAYING) {
            while (elapsed >= TICK_DURATION && !gameOver && !win) {
                globalTick++;
                if (globalTick % PAC_SPEED == 0) movePacman();
                if (globalTick % GHOST_SPEED == 0) moveGhosts();
                if (frightened) {
                    frightTimer--;
                    if (frightTimer <= 0) frightened = false;
                }
                if (dotsLeft <= 0) {
                    win = true;
                    addHighScore(score);
                }
                elapsed -= TICK_DURATION;
                lastUpdateTime += TICK_DURATION;
            }
            if (elapsed >= TICK_DURATION) lastUpdateTime = curr;
            drawScene();
        }
        else if (currentState == STATE_MAIN_MENU) {
            drawMainMenu();
        }
        else if (currentState == STATE_HELP) {
            drawHelp();
        }
        else if (currentState == STATE_ABOUT) {
            drawAbout();
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    saveHighScores();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}