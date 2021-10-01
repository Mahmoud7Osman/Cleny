#include <ncurses.h>
#include <fstream>
#include <cstring>

#define  UN_MAX   40
#define  PS_MAX   40



void GetPassword(WINDOW*, char*, int);


int main(int argc, char **argv){
    if (argc < 2){
       return printf ("Usage: authenticate  <Filename To Write Credentials>\n");
    }

    int y, x;
    char username[40], password[40];
    WINDOW* win32;
    WINDOW*   hdr;
    FILE*      fp;

    initscr();
    start_color();

    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    win32=newwin(20, 100, 10, 30);
    fp=fopen(argv[1], "a");

    box(win32, 0x00, 0x00);
    wmove(win32, 1, 85);
    waddstr(win32, "Authentication");
    wmove(win32, 5, 5);
    waddstr(win32, "Username: ");
    wmove(win32, 10, 5);
    waddstr(win32, "Password: ");
    wmove(win32, 17, 44);
    waddstr(win32, "Verify");
    wrefresh(win32);

    wmove(win32, 5, 15);
    wgetnstr(win32, username, UN_MAX);
    wmove(win32, 10, 15);
    GetPassword(win32, password, PS_MAX);
    refresh();
    touchwin(win32);

    curs_set(0);

    wmove(win32, 17, 44);
    wattron(win32, COLOR_PAIR(1));
    waddstr(win32, "Verify");
    wattroff(win32, COLOR_PAIR(1));
    wrefresh(win32);
    noecho();
    while (getch()!=0x0A) ;
    endwin();

    fwrite(username, strlen(username), 1, fp);
    fwrite("\n",          1          , 1, fp);
    fwrite(password, strlen(password), 1, fp);
    fclose(fp);

    return 0;
}

void GetPassword(WINDOW* win, char* buf, int size){
    noecho();
    for (int tmp=0; tmp<size; tmp++){
       *(buf+tmp)=wgetch(win);
       if (*(buf+tmp)==0x0A)
          break;
       waddch(win, '*');
    }
}
