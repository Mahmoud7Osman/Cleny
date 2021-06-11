#include "../lib/include/done.h"
#include "../lib/include/fatal.h"
#include <cstring>
#include <stdlib.h>

int main(int argc, char **argv){
  if (argc<2){
     fatal ("Usage: weather <countrie name>\n");
     return -1;
  }
  char c[14+strlen(argv[1])]="curl wttr.in/";
  done("Getting Weather Info...");
  strcat (c, argv[1]);
  return system(c);
}
