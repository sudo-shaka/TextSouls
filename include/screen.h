#ifndef __SCREEN__
#define __SCREEN__

#include<ncurses.h>

void initScreen();
void closeScreen();
void clearScreen();
void  renderOnTerm(char **input,int height,int width);

#endif
