#include <ncurses.h>
#include <iostream>

int main(void){
   int x, y;

   initscr();

   getmaxyx(stdscr, y, x);
   printf ("X=%d Y=%d\n", x, y);

   endwin();
}
