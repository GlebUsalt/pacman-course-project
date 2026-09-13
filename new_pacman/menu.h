#ifndef MENU_H
#define MENU_H

extern const char* menuItems[];
extern int menuSize;
extern int selectedMenuItem;

void drawMainMenu(void);
void drawHelp(void);
void drawAbout(void);

#endif
