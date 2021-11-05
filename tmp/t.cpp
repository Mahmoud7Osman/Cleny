#include <ncurses.h>

int main(){
  WINDOW* testwin;
  initscr();
  curs_set(0);
  start_color();
  testwin=newwin(30, 30, 4, 5);
  printw("%x",testwin);
  init_pair(1, COLOR_WHITE, COLOR_BLUE);
  wbkgd(testwin, COLOR_PAIR(1));
  box(testwin, 0, 0);
  touchwin(testwin);
  getch();
  scrollok(testwin, TRUE);
  wrefresh(testwin);
  getch();
  for (int i;i<=100;i++){
   wprintw(testwin,"Window %d!!\n", i);
   napms(399);
   wrefresh(testwin);
  }
  endwin();
}
