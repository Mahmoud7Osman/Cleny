#include "../lib/include/done.h"

#include <cstring>
#include <stdlib.h>

int main(int argc, char **argv){
  char c[]="curl wttr.in/";
  if (argc==1){ return printf ("Usage: weather <countrie name>\n");}
  done("Getting Weather Info...");
  strcat (c, argv[1]);
  return system(c);
}
