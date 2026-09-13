#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "highscores.h"
#include "render.h"
#include "menu.h"

#define WINDOW_WIDTH  750
#define WINDOW_HEIGHT 700

static GameState currentState = STATE_MAIN_MENU;
static Game game;
static double lastUpdateTime = 0.0;

static void keyCallback(GLFWwindow* window, int key, int scancode,
                        int action, int mods) {
    (void)scancode; (void)mods;
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
                    initGame(&game);
                    lastUpdateTime = glfwGetTime();
                } else if (selectedMenuItem == 1) {
                    currentState = STATE_HELP;
                } else if (selectedMenuItem == 2) {
                    currentState = STATE_ABOUT;
                } else if (selectedMenuItem == 3) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
        }
    } else if (currentState == STATE_PLAYING) {
        if (key == GLFW_KEY_ESCAPE) {
            currentState = STATE_MAIN_MENU;
            return;
        }
        if (!game.gameOver && !game.win) {
            switch (key) {
                case GLFW_KEY_LEFT:  case GLFW_KEY_A:
                    game.pacman.nextDRow = 0;  game.pacman.nextDCol = -1; break;
                case GLFW_KEY_RIGHT: case GLFW_KEY_D:
                    game.pacman.nextDRow = 0;  game.pacman.nextDCol = 1;  break;
                case GLFW_KEY_UP:    case GLFW_KEY_W:
                    game.pacman.nextDRow = -1; game.pacman.nextDCol = 0;  break;
                case GLFW_KEY_DOWN:  case GLFW_KEY_S:
                    game.pacman.nextDRow = 1;  game.pacman.nextDCol = 0;  break;
            }
        }
        if ((game.gameOver || game.win) && key == GLFW_KEY_R) {
            initGame(&game);
            lastUpdateTime = glfwGetTime();
        }
    } else if (currentState == STATE_HELP || currentState == STATE_ABOUT) {
        if (key == GLFW_KEY_ESCAPE) currentState = STATE_MAIN_MENU;
    }
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    if (width <= 0 || height <= 0) return;
    glViewport(0, 0, width, height);
}

/* Проекция фиксирована в координатах 750x700: сцена растягивается
 * вместе с окном, не искажая пропорции игрового поля относительно HUD. */
static void setupProjection(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void simulateLoading(GLFWwindow* window) {
    for (int p = 0; p <= 100; p++) {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        drawProgressBar(200, 400, 350, 40, (float)p, "LOADING GAME");
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwWaitEventsTimeout(0.003);
    }
}

int main(void) {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "Pac-Man 23x21", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);                       /* vsync */
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed: %s\n",
                glewGetErrorString(err));
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    setupProjection();

    loadHighScores();
    simulateLoading(window);

    while (!glfwWindowShouldClose(window)) {
        double curr = glfwGetTime();
        double elapsed = curr - lastUpdateTime;

        if (currentState == STATE_PLAYING) {
            int ticks = 0;
            while (elapsed >= TICK_DURATION &&
                   !game.gameOver && !game.win &&
                   ticks < MAX_TICKS_PER_FRAME) {
                game.globalTick++;
                if (game.globalTick % PAC_SPEED == 0)   movePacman(&game);
                if (game.globalTick % GHOST_SPEED == 0) moveGhosts(&game);

                if (game.frightened) {
                    game.frightTimer--;
                    if (game.frightTimer <= 0) game.frightened = false;
                }
                if (!game.gameOver && game.dotsLeft <= 0) {
                    game.win = true;
                    addHighScore(game.score);
                }

                elapsed -= TICK_DURATION;
                lastUpdateTime += TICK_DURATION;
                ticks++;
            }
            /* Если не успели догнать - сбрасываем отставание,
             * иначе получим "догоняющий" цикл после лага. */
            if (elapsed >= TICK_DURATION) lastUpdateTime = curr;
            drawScene(&game);
        } else if (currentState == STATE_MAIN_MENU) {
            drawMainMenu();
        } else if (currentState == STATE_HELP) {
            drawHelp();
        } else if (currentState == STATE_ABOUT) {
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
