#include <ncurses.h>
#include <iostream>
#include <cstring>

using namespace std;
char online_name[16];

static char* path(void){
      char *PODGL=getenv("PODGL");
      char *dpath=(char*)malloc(strlen(PODGL)+17);
      strcat(dpath, PODGL);
      strcat(dpath, "/etc/iChat/name.i");
      return dpath;
}

int main(void){
    initscr();
    start_color();
    init_color(1, 1000, 1000, 1900);
    init_pair(1,1, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    online_name[15]=0;
    printw("Enter Your Online Name (15 Character Max):                |");
    move(0,43);
    getnstr(online_name, 15);
    attrset(A_NORMAL);
    printw("Getting Things Ready...\n\n");
    char *dpath=path();
    FILE *fptr=fopen(dpath, "w+");
    free(dpath);
    fwrite(online_name, 1, strlen(online_name), fptr);
    printw("Done, Press C To Continue ");
    refresh();
    while (getchar()!='c')
        ;
    endwin();
    return 0;
}
