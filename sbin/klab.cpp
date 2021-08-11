#include <ncurses.h>

int PHsize=0;
int strx=0, strxt=0, stry;

void outkey(int key){
   int y, x;
   getyx(stdscr, y, x);
   mvprintw((++y+1), (x-3),"%c", key);
   move(--y,x);
}

void outsize(){
    int y, x, oy, ox;
    getmaxyx(stdscr, y, x);
    getyx(stdscr, oy, ox);
    PHsize++;
    mvprintw(0,(x-20),"Size in Bytes: %d", PHsize);
    move(oy, ox);
}
void outstr(int key){
    getyx(stdscr,stry ,strx);

    move(stry+10,strxt);
    addch(key);
    move(stry, strx);
    strxt++;
}
int main(void){
   printf ("Ready To Start Typing ...!\n");
   initscr();

   int key;

   printw ("Press Enter Or Backspace To Stop\n\n\n\n\n\n");
   while  ( (key=getchar()) !=0x0D && key!=0x08){
     printw("%x ",key);
     outkey(key);
     outstr(key);
     outsize();
     refresh();
   }

   endwin();
   return 0;
}
