#include <ncurses.h>

void initScreen(){
  initscr();
  noecho();
  curs_set(0);
  refresh();
}

void clearScreen(){
  clear();
  refresh();
}

void  renderOnTerm(char **input,int height,int width){
  for(int i=0;i<width;i++){
    for(int j=0;j<height;j++){
      mvaddch(i,j,input[i][j]);
    }
  }
  refresh();
}
