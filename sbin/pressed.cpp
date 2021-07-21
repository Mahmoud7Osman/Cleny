#include <ncurses.h>

int main(int argc, char **argv){
    initscr();
    while (getchar()!=*argv[1])
         ;
    return endwin();
}
