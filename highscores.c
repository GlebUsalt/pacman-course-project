#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "constants.h"
#include "highscores.h"

int highScores[MAX_SCORES] = { 0 };
int numScores = 0;

void loadHighScores() {
    FILE* f = fopen("highscores.txt", "r");
    if (f) {
        numScores = 0;
        while (numScores < MAX_SCORES && fscanf(f, "%d", &highScores[numScores]) == 1) {
            numScores++;
        }
        fclose(f);
    }
    else {
        numScores = 0;
    }
}

void saveHighScores() {
    FILE* f = fopen("highscores.txt", "w");
    if (f) {
        for (int i = 0; i < numScores; i++) {
            fprintf(f, "%d\n", highScores[i]);
        }
        fclose(f);
    }
}

void addHighScore(int newScore) {
    int temp[MAX_SCORES + 1];
    for (int i = 0; i < numScores; i++) temp[i] = highScores[i];
    temp[numScores] = newScore;
    numScores++;
    for (int i = 0; i < numScores - 1; i++) {
        for (int j = i + 1; j < numScores; j++) {
            if (temp[i] < temp[j]) {
                int t = temp[i];
                temp[i] = temp[j];
                temp[j] = t;
            }
        }
    }
    if (numScores > MAX_SCORES) numScores = MAX_SCORES;
    for (int i = 0; i < numScores; i++) highScores[i] = temp[i];
    saveHighScores();
}