#include <ncurses.h>
#include <iostream>
#include "../lib/include/colors.h"

int main(int argc, char ** argv)
{
	initscr();
	printf("%s ? [%sY/n%s] ",argv[1], KCYN, KWHT);

        int val=0;

        while (1)
        {
           int ch=getchar();
           if (ch=='Y' | ch=='y'){val=1; break;}
           if (ch=='n' | ch=='N'){val=0; break;}

        }

        printf("\n");

        endwin();
        return val;
}
