#include "highscores.h"
#include <stdio.h>
#include <stdlib.h>

int highScores[MAX_SCORES] = { 0 };
int numScores = 0;

void loadHighScores(void) {
    FILE* f = fopen("highscores.txt", "r");
    if (!f) { numScores = 0; return; }
    numScores = 0;
    while (numScores < MAX_SCORES &&
           fscanf(f, "%d", &highScores[numScores]) == 1) {
        numScores++;
    }
    fclose(f);
}

void saveHighScores(void) {
    FILE* f = fopen("highscores.txt", "w");
    if (!f) return;
    for (int i = 0; i < numScores; i++) {
        fprintf(f, "%d\n", highScores[i]);
    }
    fclose(f);
}

static int compareDesc(const void* a, const void* b) {
    int va = *(const int*)a;
    int vb = *(const int*)b;
    return (vb > va) - (vb < va);
}

void addHighScore(int newScore) {
    if (numScores < MAX_SCORES) {
        highScores[numScores++] = newScore;
    } else if (newScore > highScores[MAX_SCORES - 1]) {
        highScores[MAX_SCORES - 1] = newScore;
    } else {
        return;
    }
    qsort(highScores, numScores, sizeof(int), compareDesc);
    saveHighScores();
}
