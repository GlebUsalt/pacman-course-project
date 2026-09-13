#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#define MAX_SCORES 10

extern int highScores[MAX_SCORES];
extern int numScores;

void loadHighScores(void);
void saveHighScores(void);
void addHighScore(int newScore);

#endif
