#include <ncurses.h>

void initScreen(){
  initscr();
  noecho();
  curs_set(0);
  refresh();
}

void closeScreen(){
  endwin();
}

void clearScreen(){
  clear();
  refresh();
}

void  renderOnTerm(char **input,int height,int width){
  noecho();
  curs_set(0);
  for(int i=0;i<width;i++){
    for(int j=0;j<height;j++){
      mvaddch(i,j,input[i][j]);
    }
  }
  refresh();
}
