#include <iostream>
#include <cstring>

int main(int argc, char**argv){
    char crusor[13]="\033[";
    if (argc!=3){
      return printf ("Usage: move <X> <Y>\n");
    }
    strncat(crusor, argv[1], 3);
    strcat(crusor, ";");
    strncat(crusor, argv[2], 3);
    strcat(crusor, "H");
    printf (crusor);
}
