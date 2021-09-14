#include <cstring>
#include <iostream>

int main(int argc, char**argv){
   long len=0;
   if (argc < 2) return printf ("Error: No Arguments\n");
   for (int tmp=1; tmp<argc; tmp++)
      len+=strlen(argv[tmp]);
   return printf ("%d\n", len);
}
